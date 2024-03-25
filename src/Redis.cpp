#include <opencv2/core.hpp>
#include "Redis.hpp"

Redis::Redis(const std::string& host, int port) : 
  redis_(sw::redis::Redis("tcp://" + host + ":" + std::to_string(port))) {}

Redis::~Redis() {}

void Redis::sendDict(const std::vector<int>& classIds, const std::vector<cv::Rect>& bboxes, const std::vector<float>& confidences)
{
  auto startTime = std::chrono::high_resolution_clock::now();
  using Attrs = std::unordered_map<std::string, std::string>;
  auto pipe = redis_.pipeline(false);
  for (int i = 0; i < classIds.size(); ++i)
  {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "[%d,%d,%d,%d]", bboxes[i].x, bboxes[i].y, bboxes[i].width, bboxes[i].height);
    Attrs attrs = {
      {"classId", std::to_string(classIds[i])},
      {"confidence", std::to_string(confidences[i])},
      {"bbox",  buffer}
    };
    pipe.xadd("test", "*", attrs.begin(), attrs.end());
  }
  pipe.exec();

  t.insertTimeDifference(startTime);
  emit updateTimer(t.average());
}