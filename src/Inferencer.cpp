#include "Inferencer.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <opencv2/imgproc.hpp>

std::random_device rd;  // Obtain a random number from hardware
std::mt19937 gen(rd()); // Seed the generator
std::uniform_int_distribution<> distr(10, 20); // Define the range

Inferencer::Inferencer(QObject *parent): QThread(parent)
{
  std::string onnxFpath = "./models/best.onnx";
  Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "DetectionInference");
  Ort::SessionOptions sessionOps;
  sessionOps.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
  sessionOps.SetIntraOpNumThreads(4);
  session = new Ort::Session(env, onnxFpath.c_str(), sessionOps);

}

Inferencer::~Inferencer()
{
  session->release();
}

// float *preprocess(cv::Mat &mat)
// {
//   float *inputTensor;

//   return inputTensor;
// }

void Inferencer::doInference(const cv::Mat &frame)
{
  std::cout << "Inferencer::doInference\n";
  
  // Simulate inference
  std::this_thread::sleep_for(std::chrono::milliseconds(distr(gen)));
  cv::putText(frame, "watermark", cv::Point(20, 30), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0), 2, 0);

  emit frameProcessed(frame);
}

// void Inferencer::run(void)
// {

// }

// void Inferencer::work(void)
// {

// }

// float *Inferencer::preprocess(void)
// {

// }
