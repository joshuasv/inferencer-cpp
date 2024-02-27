#ifndef INFERENCER_HPP
#define INFERENCER_HPP

#include <QThread>
#include <opencv2/core.hpp>
#include "onnxruntime/onnxruntime_cxx_api.h"

class Inferencer: public QThread
{
  Q_OBJECT

  public:
    Inferencer(QObject *parent = nullptr);
    ~Inferencer();
    void doInference(const cv::Mat &frame);

  private:
  //   void run(void);
  //   void work(void);
  //   float *preprocess(cv::Mat &mat);

    Ort::Session *session;

  signals:
    void frameProcessed(const cv::Mat &mat); 
};

#endif // INFERENCER_HPP