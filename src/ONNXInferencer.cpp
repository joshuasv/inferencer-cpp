#include "ONNXInferencer.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <numeric>
#include <chrono>
#include <thread>

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
    outputNodeShape = tensorInfo.GetShape();
  }
}

ONNXInferencer::~ONNXInferencer()
{
  #ifdef DEBUG_MODE
  // Save timer measurements to file
  t.saveMeasurements("inferencer_times");
  #endif
}

Ort::Session ONNXInferencer::createSession(const Ort::Env& env, const std::string& modelFPath)
{
  Ort::SessionOptions sessOps;
  sessOps.SetExecutionMode(ORT_SEQUENTIAL);
  sessOps.SetGraphOptimizationLevel(ORT_ENABLE_ALL);
  sessOps.SetIntraOpNumThreads(4);
  sessOps.AddConfigEntry("kOrtSessionOptionsConfigAllowIntraOpSpinning", "0");
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
  try
  {
    // TODO: refactor
    Ort::Value outputTensor{nullptr};
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
    session.Run(
      Ort::RunOptions{nullptr},
      inputNodeNames.data(),
      &inputTensor,
      1,
      outputNodeNames.data(),
      &outputTensor,
      1
    );
    // Grab output bboxes
    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    float* output = outputTensor.GetTensorMutableData<float>();
    outputData = cv::Mat(outputNodeShape.at(1), outputNodeShape.at(2), CV_32F, output);
    outputData = outputData.t();
    data = (float*) outputData.data;
    for (int i = 0; i < outputData.rows; ++i)
    {
      float* classesScores = data + 4;
      cv::Mat scores(1, this->classes.size(), CV_32FC1, classesScores);
      cv::Point class_id;
      double maxClassScore;
      cv::minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);
      if (maxClassScore > rectConfidenceThreshold)
      {
        confidences.push_back(maxClassScore);
        class_ids.push_back(class_id.x);
        float x = data[0];
        float y = data[1];
        float w = data[2];
        float h = data[3];
        int left = int((x - 0.5 * w) * factorX);
        int top = int((y - 0.5 * h) * factorY);
        int width = int(w * factorX);
        int height = int(h * factorY);
        boxes.push_back(cv::Rect(left, top, width, height));
      }
      data += outputData.cols;
    }
    // Do NMS
    std::vector<int> nmsResult;
    cv::dnn::NMSBoxes(boxes, confidences, rectConfidenceThreshold, iouThreshold, nmsResult);
    std::vector<int> classIds;
    std::vector<cv::Rect> bboxes;
    std::vector<float> confidences_;
    for (int i = 0; i < nmsResult.size(); ++i)
    {
      int idx = nmsResult[i];
      int classId = class_ids[idx];
      float confidence = confidences[idx];
      cv::Rect bbox = boxes[idx];
      classIds.push_back(classId);
      bboxes.push_back(bbox);
      confidences_.push_back(confidence);
    }

    emit resultsReady(frame, classIds, bboxes, confidences_);

    t.insertTimeDifference();
    emit updateTimer(t.average());
  }
  catch (const std::exception& e)
  {
    std::cerr << "Exception caught in event handler:" << e.what();
  }
}