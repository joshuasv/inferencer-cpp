#ifndef WORKER_HPP
#define WORKER_HPP

#include <random>
#include <QThread>
#include <opencv2/opencv.hpp>
#include "globals.hpp"

class Worker: public QThread
{
  Q_OBJECT
  
  public:
    Worker(QObject *parent = nullptr);
    ~Worker();

  private:
    cv::VideoCapture cap;
    void run(void);
    void work(void);

  signals:
    // void result(const int &N);
    // void timeOut(void);
    void frameCaptured(const cv::Mat &frame);
};

#endif  // WORKER_HPP