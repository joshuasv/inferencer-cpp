#ifndef WORKER_HPP
#define WORKER_HPP

#include <random>
#include <QThread>
#include <opencv2/opencv.hpp>
#include "MyTimer.hpp"

class Worker: public QThread
{
  Q_OBJECT
  
  public:
    Worker(int source, QObject *parent = nullptr);
    Worker(std::string source, QObject *parent = nullptr);
    ~Worker();

  private:
    cv::VideoCapture cap;
    void run(void);
    MyTimer t = MyTimer();
    int sourceFPS;
    bool isFile = false;
    int frameDelay = 0;

  signals:
    void frameCaptured(const cv::Mat &frame);
    void updateTimer(const float& value);
};

#endif  // WORKER_HPP