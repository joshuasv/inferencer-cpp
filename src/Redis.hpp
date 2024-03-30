#ifndef REDIS_HPP
#define REDIS_HPP

#include <string>
#include <vector>
#include <sw/redis++/redis++.h>
#include <QThread>
#include <opencv2/core.hpp>
#include "MyTimer.hpp"

class Redis: public QThread
{
  Q_OBJECT

  public:
    Redis(const std::string& host, int port);
    ~Redis();
    void sendDict(const std::vector<int>& classIds, const std::vector<cv::Rect>& bboxes, const std::vector<float>& confidences);
  
  private:
    sw::redis::Redis redis_;
    MyTimer t = MyTimer();
  
  signals:
    void updateTimer(const float& value);
};

#endif //REDIS_HPP