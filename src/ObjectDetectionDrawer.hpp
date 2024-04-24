#ifndef OBJECTDETECTIONDRAWER_HPP
#define OBJECTDETECTIONDRAWER_HPP

#include <vector>
#include <string>
#include <QThread>
#include <Eigen/Dense>
#include <opencv2/core.hpp>
#include "MyTimer.hpp"

class ObjectDetectionDrawer: public QThread
{
  Q_OBJECT

  public:
    ObjectDetectionDrawer(const std::vector<std::string>& classNames, const std::vector<cv::Scalar>& classColors, QObject* parent = nullptr);
    ~ObjectDetectionDrawer();

  private:
    const std::vector<std::string> classNames;
    const std::vector<cv::Scalar> classColors;
    MyTimer t = MyTimer();

  signals:
    void frameReady(const cv::Mat& frame);
    void updateTimer(const float& value);

  public slots:
    void draw(const cv::Mat& frame, const std::vector<int>& classIds, const Eigen::MatrixXf& tlbrBoxes, const Eigen::Map<Eigen::MatrixXf>& confidences, const std::vector<int>& trackIds);
    
};

#endif // OBJECTDETECTIONDRAWER_HPP