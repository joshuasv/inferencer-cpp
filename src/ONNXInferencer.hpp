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
    int inputNodeBytes;
    Ort::Value& preprocessFrame(const cv::Mat& inFrame);
    Ort::Session createSession(const Ort::Env& env, const std::string& modelFPath);

  signals:
    void frameReady(const cv::Mat& frame);

  public slots:
    void runInference(const cv::Mat& frame);

};

#endif // ONNXINFERENCER_HPP