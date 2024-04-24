#include "ONNXInferencer.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <numeric>
#include <chrono>
#include <thread>

int vectorProduct(const std::vector<int64_t>& v)
{
  return accumulate(v.begin(), v.end(), 1, std::multiplies<int64_t>());
}

ONNXInferencer::ONNXInferencer(const std::string& modelFPath, QObject* parent) 
  : QThread(parent),
    env(ORT_LOGGING_LEVEL_WARNING, "test"), 
    session(createSession(env, modelFPath)),
    memInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)
  )
{
  // Get input and output node names
  for (size_t i = 0; i < session.GetInputCount(); ++i)
  {
    Ort::AllocatedStringPtr nodeName = session.GetInputNameAllocated(i, defAllocator);
    char* tmpBuff = new char[50];
    strcpy(tmpBuff, nodeName.get());
    inputNodeNames.push_back(tmpBuff);
    // Get input shape
    Ort::TypeInfo typeInfo = session.GetInputTypeInfo(i);
    auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
    inputNodeShape = tensorInfo.GetShape();
    inputNodeBytes = std::accumulate(inputNodeShape.begin(), inputNodeShape.end(), 1, std::multiplies<int64_t>());
  }
  for (size_t i = 0; i < session.GetOutputCount(); ++i)
  {
    char* tmpBuff = new char[50];
    Ort::AllocatedStringPtr nodeName = session.GetOutputNameAllocated(i, defAllocator);
    strcpy(tmpBuff, nodeName.get());
    outputNodeNames.push_back(tmpBuff);
    // Get output shape
    Ort::TypeInfo typeInfo = session.GetOutputTypeInfo(i);
    auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
    // outputNodeShape = tensorInfo.GetShape();
    outputNodeShapes.push_back(tensorInfo.GetShape());
  }
}

ONNXInferencer::~ONNXInferencer()
{
  #ifdef DEBUG_MODE
  // Save timer measurements to file
  timer.saveMeasurements("inferencer_times");
  #endif
}

Ort::Session ONNXInferencer::createSession(const Ort::Env& env, const std::string& modelFPath)
{
  Ort::SessionOptions sessOps;
  sessOps.SetExecutionMode(ORT_SEQUENTIAL);
  sessOps.SetGraphOptimizationLevel(ORT_ENABLE_ALL);
  // sessOps.SetIntraOpNumThreads(2);
  // sessOps.SetInterOpNumThreads(1);
  // sessOps.AddConfigEntry("kOrtSessionOptionsConfigAllowIntraOpSpinning", "0");
  // sessOps.AddConfigEntry("kOrtSessionOptionsConfigIntraOpThreadAffinities", "3,4");

  return Ort::Session(env, modelFPath.c_str(), sessOps);
}

// Ort::Value& ONNXInferencer::preprocessFrame(const cv::Mat& inFrame)
// {
//   cv::Mat tmp;
//   cv::dnn::blobFromImage(inFrame, tmp, 1./255., cv::Size(inputNodeShape.at(3), inputNodeShape.at(2)), 0.0, true, false, CV_32F);
//   Ort::Value outTensor = Ort::Value::CreateTensor<float>(
//     memInfo,
//     (float*) tmp.data,
//     inputNodeBytes,
//     inputNodeShape.data(),
//     inputNodeShape.size()
//   );

//   return outTensor;
// }

void ONNXInferencer::runInference(const cv::Mat& frame)
{
  auto startTime = std::chrono::high_resolution_clock::now();
  // TODO: refactor
  // Ort::Value outputTensor{nullptr};
  cv::Mat outputData;
  float* data;

  cv::Mat tmp; //, tmpDisplay;
  // frame.copyTo(tmp);
  tmp = frame.clone();
  // frame.copyTo(tmpDisplay);

  // Preprocess frame
  cv::dnn::blobFromImage(tmp, tmp, 1./255., cv::Size(inputNodeShape.at(3), inputNodeShape.at(2)), 0.0, true, false, CV_32F);
  Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
    memInfo,
    (float*) tmp.data,
    inputNodeBytes,
    inputNodeShape.data(),
    inputNodeShape.size()
  );

  // Do inference
  std::vector<Ort::Value> outputTensors = session.Run(
    Ort::RunOptions{nullptr},
    inputNodeNames.data(),
    &inputTensor,
    1,
    outputNodeNames.data(),
    // &outputTensor,
    3
  );

  // Get output tensor information since we have dynamic axes
  auto& classIdsTensor = outputTensors[0];
  auto& confidencesTensor = outputTensors[1];
  auto& bboxesTensor = outputTensors[2];
  auto classIdsTensorInfo = classIdsTensor.GetTensorTypeAndShapeInfo();
  auto confidencesTensorInfo = confidencesTensor.GetTensorTypeAndShapeInfo();
  auto bboxesTensorInfo = bboxesTensor.GetTensorTypeAndShapeInfo();
  std::vector<int64_t> classIdsDims = classIdsTensorInfo.GetShape();
  std::vector<int64_t> confidencesDims = confidencesTensorInfo.GetShape();
  std::vector<int64_t> bboxesDims = bboxesTensorInfo.GetShape();
  int classIdsNumElements = vectorProduct(classIdsDims);
  int confidencesNumElements = vectorProduct(confidencesDims);
  int bboxesNumElements = vectorProduct(bboxesDims);
  int64_t* classIds_ = classIdsTensor.GetTensorMutableData<int64_t>();
  std::vector<int64_t> classIds64(classIds_, classIds_ + classIdsNumElements);
  std::vector<int> classIds;
  classIds.reserve(classIdsNumElements);
  for (int64_t value: classIds64)
  {
    classIds.push_back(static_cast<int>(value));
  }
  float* confidences_ = confidencesTensor.GetTensorMutableData<float>();
  float* bboxes_ = bboxesTensor.GetTensorMutableData<float>();

  // Map network outputs to Eigen matrices
  Eigen::Map<Eigen::MatrixXf> confidences(confidences_, bboxesDims.at(0), 1);
  Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, 4, Eigen::RowMajor>> boxesMap(bboxes_, bboxesDims.at(0), 4);
  Eigen::MatrixXf boxesProbs(bboxesDims.at(0), 5);
  Eigen::MatrixXf tlbrBoxes(boxesMap.rows(), 4);
  Eigen::VectorXf cx = boxesMap.col(0).eval();
  Eigen::VectorXf cy = boxesMap.col(1).eval();
  Eigen::VectorXf w = boxesMap.col(2).eval();
  Eigen::VectorXf h = boxesMap.col(3).eval();
  Eigen::VectorXf t = cy - (0.5 * h).eval();
  Eigen::VectorXf l = cx - (0.5 * w).eval();
  Eigen::VectorXf b = cy + (0.5 * h).eval();
  Eigen::VectorXf r = cx + (0.5 * w).eval();
  t = (t * factorY).eval();
  l = (l * factorX).eval();
  b = (b * factorY).eval();
  r = (r * factorX).eval();
  tlbrBoxes << t, l, b, r;
  boxesMap.col(0) = t; boxesMap.col(1) = l;
  boxesProbs << boxesMap, confidences;

  // Do tracking
  std::vector<KalmanBBoxTrack>tracks = tracker.process_frame_detections(boxesProbs);
  std::vector<int> trackIds = match_detections_with_tracks(tlbrBoxes.cast<double>(), tracks);

  emit resultsReady(frame, classIds, tlbrBoxes, confidences, trackIds);
  emit sendResults(classIds, confidences, trackIds);
 
  timer.insertTimeDifference(startTime);
  emit updateTimer(timer.average());
}