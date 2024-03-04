#ifndef ONNXINFERENCER_HPP
#define ONNXINFERENCER_HPP

#include <QThread>
#include <opencv2/core.hpp>
#include "onnxruntime/onnxruntime_cxx_api.h"


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
    std::vector<int64_t> outputNodeShape;
    int inputNodeBytes;
    Ort::Value& preprocessFrame(const cv::Mat& inFrame);
    Ort::Session createSession(const Ort::Env& env, const std::string& modelFPath);
    // TODO: refactor
    float rectConfidenceThreshold = 0.45;
    float iouThreshold = 0.5;
    float factorX = 1.0;
    float factorY = 0.75;
    std::vector<std::string> classes{"white", "blue", "orange"};
    std::vector<cv::Scalar> colors{cv::Scalar(255, 255, 255), cv::Scalar(165, 0, 0), cv::Scalar(0, 165, 255)};

  signals:
    void frameReady(const cv::Mat& frame);

  public slots:
    void runInference(const cv::Mat& frame);

};

#endif // ONNXINFERENCER_HPP