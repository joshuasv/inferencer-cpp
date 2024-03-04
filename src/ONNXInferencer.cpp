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
  }
}

ONNXInferencer::~ONNXInferencer()
{

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

Ort::Value& ONNXInferencer::preprocessFrame(const cv::Mat& inFrame)
{
  cv::Mat tmp;
  cv::dnn::blobFromImage(inFrame, tmp, 1./255., cv::Size(inputNodeShape.at(3), inputNodeShape.at(2)), 0.0, true, false, CV_32F);
  Ort::Value outTensor = Ort::Value::CreateTensor<float>(
    memInfo,
    (float*) tmp.data,
    inputNodeBytes,
    inputNodeShape.data(),
    inputNodeShape.size()
  );

  return outTensor;
}

void ONNXInferencer::runInference(const cv::Mat& frame)
{
  Ort::Value outputTensor{nullptr};
  // Preprocess frame
  cv::Mat tmp, tmpDisplay;
  frame.copyTo(tmp);
  frame.copyTo(tmpDisplay);
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
  
  // TOREMOVE;
  cv::putText(tmpDisplay, "watermark", cv::Point(50,50), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0,0,0), 5);

  emit frameReady(tmpDisplay);
}