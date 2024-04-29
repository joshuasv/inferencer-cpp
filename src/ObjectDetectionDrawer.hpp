#ifndef OBJECTDETECTIONDRAWER_HPP
#define OBJECTDETECTIONDRAWER_HPP

#include <vector>
#include <string>
#include <QThread>
#include <Eigen/Dense>
#include <opencv2/core.hpp>

#include "Redis.hpp"
#include "MyTimer.hpp"

class ObjectDetectionDrawer: public QThread
{
  Q_OBJECT

  public:
    ObjectDetectionDrawer(const std::vector<std::string>& classNames, const std::vector<cv::Scalar>& classColors, Redis* redis, const std::string& channelName, QObject* parent = nullptr);
    ~ObjectDetectionDrawer();

  private:
    MyTimer t = MyTimer();
    void subscribeFeedbackChannel_();
    bool isClassIdAccepted_(int classId);
    const std::vector<std::string> classNames;
    const std::vector<cv::Scalar> classColors;
    Redis* redis_;
    const std::string& channelName_;
    QThread* feedbackThread_;
    std::vector<int> acceptedClassIds_;

  signals:
    void frameReady(const cv::Mat& frame);
    void updateTimer(const float& value);

  public slots:
    void draw(const cv::Mat& frame, const std::vector<int>& classIds, const Eigen::MatrixXf& tlbrBoxes, const Eigen::Map<Eigen::MatrixXf>& confidences, const std::vector<int>& trackIds);
    
};

#endif // OBJECTDETECTIONDRAWER_HPP