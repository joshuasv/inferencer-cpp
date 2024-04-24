#include "ObjectDetectionDrawer.hpp"

#include <iostream>
#include <opencv2/imgproc.hpp>

ObjectDetectionDrawer::ObjectDetectionDrawer(const std::vector<std::string>& classNames, const std::vector<cv::Scalar>& classColors, QObject* parent)
  : QThread(parent), classNames(classNames), classColors(classColors) { }

ObjectDetectionDrawer::~ObjectDetectionDrawer()
{
  #ifdef DEBUG_MODE
  // Save timer measurements to file
  t.saveMeasurements("drawer_times");
  #endif
}

void ObjectDetectionDrawer::draw(const cv::Mat& frame, const std::vector<int>& classIds, const Eigen::MatrixXf& tlbrBoxes, const Eigen::Map<Eigen::MatrixXf>& confidences, const std::vector<int>& trackIds)
{
  auto startTime = std::chrono::high_resolution_clock::now();
  
  cv::Mat frameDraw = frame.clone();
  if (classIds.size() > 0)
  {
    for (int i = 0; i < classIds.size(); ++i)
    {
      int classId = classIds.at(i);
      // cv::Rect bbox = boxesCv.at(i);
      auto bbox = tlbrBoxes.row(i);
      int bboxX1 = bbox(0, 1);
      int bboxY1 = bbox(0, 0);
      int bboxX2 = bbox(0, 3);
      int bboxY2 = bbox(0, 2);
      
      
      float confidence = confidences(i, 0);
      std::string className = classNames.at(classId);
      cv::Scalar classColor = classColors.at(classId);
      std::string classString = "ID: " + std::to_string(trackIds.at(i)) + " " + className + " " + std::to_string(confidence).substr(0, 4);
      cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 1, 2, 0);
      cv::Rect textBbox = cv::Rect(bboxX1, bboxY1 - 40, textSize.width + 10, textSize.height + 20);

      cv::rectangle(frameDraw, cv::Point(bboxX1, bboxY1), cv::Point(bboxX2, bboxY2), classColor, 2);
      cv::rectangle(frameDraw, textBbox, classColor, cv::FILLED);
      cv::putText(frameDraw, classString, cv::Point(bboxX1 + 5, bboxY1 - 10), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 2, 0);
    }
  }

  emit frameReady(frameDraw);

  t.insertTimeDifference(startTime);
  emit updateTimer(t.average());
}