#ifndef ONNXINFERENCER_HPP
#define ONNXINFERENCER_HPP

#include <QThread>
#include <Eigen/Dense>
#include <opencv2/core.hpp>
#include "onnxruntime/onnxruntime_cxx_api.h"
#include "MyTimer.hpp"
#include "BYTETracker.h"

class ONNXInferencer: public QThread
{
  Q_OBJECT

  public:
    ONNXInferencer(const std::string& modelFPath, QObject* parent = nullptr);
    ~ONNXInferencer();

  private:
    Ort::Env env;
    Ort::Session session;
    Ort::MemoryInfo memInfo;
    Ort::AllocatorWithDefaultOptions defAllocator;
    std::vector<const char*> inputNodeNames;
    std::vector<const char*> outputNodeNames;
    std::vector<int64_t> inputNodeShape;
    // std::vector<int64_t> outputNodeShape;
    std::vector<std::vector<int64_t>> outputNodeShapes;
    int inputNodeBytes;
    // Ort::Value& preprocessFrame(const cv::Mat& inFrame);
    Ort::Session createSession(const Ort::Env& env, const std::string& modelFPath);
    // TODO: refactor
    float rectConfidenceThreshold = 0.45;
    float iouThreshold = 0.5;
    float factorX = 1.0;
    float factorY = 0.75;
    std::vector<std::string> classes{"white", "blue", "orange"};
    std::vector<cv::Scalar> colors{cv::Scalar(255, 255, 255), cv::Scalar(165, 0, 0), cv::Scalar(0, 165, 255)};
    MyTimer timer = MyTimer();
    BYTETracker tracker = BYTETracker(0.25, 30, 0.8, 30);

  signals:
    void resultsReady(const cv::Mat& frame, const std::vector<int>& classIds, const Eigen::MatrixXf& tlbrBoxes, const Eigen::Map<Eigen::MatrixXf>& confidences, const std::vector<int>& trackIds);
    void sendResults(const std::vector<int>& classIds, const Eigen::Map<Eigen::MatrixXf>& confidences, const std::vector<int>& trackIds);
    void updateTimer(const float& value);

  public slots:
    void runInference(const cv::Mat& frame);

};

#endif // ONNXINFERENCER_HPP