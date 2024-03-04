#ifndef WORKER_HPP
#define WORKER_HPP

#include <random>
#include <QThread>
#include <opencv2/opencv.hpp>

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

  signals:
    void frameCaptured(const cv::Mat &frame);
};

#endif  // WORKER_HPP