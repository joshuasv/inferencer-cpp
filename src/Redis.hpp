#ifndef REDIS_HPP
#define REDIS_HPP

#include <string>
#include <vector>
#include <Eigen/Dense>
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
    void sendDict(const std::vector<int>& classIds, const Eigen::Map<Eigen::MatrixXf>& confidences, const std::vector<int>& trackIds);
    sw::redis::Redis redis_;
  
  private:
    MyTimer t = MyTimer();
  
  signals:
    void updateTimer(const float& value);
};

#endif //REDIS_HPP