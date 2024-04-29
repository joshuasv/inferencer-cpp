#include "ObjectDetectionDrawer.hpp"

#include <iostream>
#include <opencv2/imgproc.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ObjectDetectionDrawer::ObjectDetectionDrawer(
  const std::vector<std::string>& classNames,
  const std::vector<cv::Scalar>& classColors,
  Redis* redis,
  const std::string& channelName,
  QObject* parent
):
  QThread(parent), classNames(classNames), classColors(classColors), 
  redis_(redis), channelName_(channelName)
{
  // Start subscriber thread to Redis channel about current order info
  acceptedClassIds_ = std::vector<int>();
  try
  {
    feedbackThread_ = QThread::create([&]() { subscribeFeedbackChannel_(); });
    if (feedbackThread_)
    {
      feedbackThread_->start();
    }
    else
    {
      throw std::runtime_error("Failed to create feedback thread.");
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error:" << e.what() << std::endl;
  }
}

ObjectDetectionDrawer::~ObjectDetectionDrawer()
{
  #ifdef DEBUG_MODE
  // Save timer measurements to file
  t.saveMeasurements("drawer_times");
  #endif

  if (feedbackThread_ && feedbackThread_->isRunning())
  {
    feedbackThread_->quit();
    feedbackThread_->wait();
    delete feedbackThread_;
    feedbackThread_ = nullptr;
  }
}

void ObjectDetectionDrawer::subscribeFeedbackChannel_()
{
  try
  {
    auto sub = redis_->redis_.subscriber();
    sub.on_message([&](std::string channel, std::string msg) {
      std::cout << "Received message from " << channel << ": " << msg << std::endl;
      if (channel == "order_updates")
      {
        acceptedClassIds_ = std::vector<int>();
        if (msg != "")
        {
          json j = json::parse(msg);
          for (const auto& obj: j["objects"])
          {
            acceptedClassIds_.push_back(obj["class_id"]);
          }
        }
      }
    });
    sub.subscribe(channelName_);
    while (true)
    {
      sub.consume();
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Redis error:" << e.what() << std::endl;
  }
}

bool ObjectDetectionDrawer::isClassIdAccepted_(int classId)
{
  if (acceptedClassIds_.empty())
  {
    return true;
  }
  else
  {
    for (int accClassId: acceptedClassIds_)
    {
      if (accClassId == classId)
      {
        return true;
      }
    }
  }
  
  return false;
}

void ObjectDetectionDrawer::draw(const cv::Mat& frame, const std::vector<int>& classIds, const Eigen::MatrixXf& tlbrBoxes, const Eigen::Map<Eigen::MatrixXf>& confidences, const std::vector<int>& trackIds)
{
  auto startTime = std::chrono::high_resolution_clock::now();

  std::cout << "Accepted classIds: ";
  for (int classId: acceptedClassIds_)
  {
    std::cout << classId << " ";
  }
  std::cout << std::endl;
  
  cv::Mat frameDraw = frame.clone();
  if (classIds.size() > 0)
  {
    for (int i = 0; i < classIds.size(); ++i)
    {
      int classId = classIds.at(i);
      bool isAccepted = isClassIdAccepted_(classId);
      auto bbox = tlbrBoxes.row(i);
      int bboxX1 = bbox(0, 1);
      int bboxY1 = bbox(0, 0);
      int bboxX2 = bbox(0, 3);
      int bboxY2 = bbox(0, 2);
      
      float confidence = confidences(i, 0);
      std::string className = classNames.at(classId);
      cv::Scalar classColor = isAccepted ? classColors.at(classId) : cv::Scalar(105, 105, 105);
      std::string classString = "ID: " + std::to_string(trackIds.at(i)) + " " + className + " " + std::to_string(confidence).substr(0, 4);
      cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 1, 2, 0);
      cv::Rect textBbox = cv::Rect(bboxX1, bboxY1 - 40, textSize.width + 10, textSize.height + 20);

      cv::rectangle(frameDraw, cv::Point(bboxX1, bboxY1), cv::Point(bboxX2, bboxY2), classColor, 2);
      cv::rectangle(frameDraw, textBbox, classColor, cv::FILLED);
      cv::putText(frameDraw, classString, cv::Point(bboxX1 + 5, bboxY1 - 10), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 2, 0);

      if (!isAccepted)
      {
        cv::line(frameDraw, cv::Point(bboxX1, bboxY1), cv::Point(bboxX2, bboxY2), classColor, 2); // From top-left to bottom-right
        cv::line(frameDraw, cv::Point(bboxX1, bboxY2), cv::Point(bboxX2, bboxY1), classColor, 2); // From bottom-left to top-right
      }
    }
  }

  emit frameReady(frameDraw);

  t.insertTimeDifference(startTime);
  emit updateTimer(t.average());
}