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

void ObjectDetectionDrawer::draw(const cv::Mat& frame, const std::vector<int>& classIds, const std::vector<cv::Rect>& bboxes, const std::vector<float>& confidences)
{
  cv::Mat frameDraw = frame.clone();
  for (int i = 0; i < classIds.size(); ++i)
  {
    int classId = classIds.at(i);
    cv::Rect bbox = bboxes.at(i);
    float confidence = confidences.at(i);
    std::string className = classNames.at(classId);
    cv::Scalar classColor = classColors.at(classId);
    std::string classString = className + " " + std::to_string(confidence).substr(0, 4);
    cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 1, 2, 0);
    cv::Rect textBbox = cv::Rect(bbox.x, bbox.y - 40, textSize.width + 10, textSize.height + 20);

    cv::rectangle(frameDraw, bbox, classColor, 2);
    cv::rectangle(frameDraw, textBbox, classColor, cv::FILLED);
    cv::putText(frameDraw, classString, cv::Point(bbox.x + 5, bbox.y - 10), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 2, 0);
  }

  emit frameReady(frameDraw);

  t.insertTimeDifference();
  emit updateTimer(t.average());
}