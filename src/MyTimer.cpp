#include <iostream>
#include <numeric>
#include "MyTimer.hpp"

MyTimer::MyTimer(size_t size) 
  : buffer(size), head(0), tail(0), isFull(false), 
    lastInsertTime(std::chrono::high_resolution_clock::now()) {}

void MyTimer::insertTimeDifference()
{
  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float, std::milli> timeDiff = now - lastInsertTime;
  insertWithTimestamp(timeDiff.count(), now);
}

void MyTimer::insertTimeDifference(std::chrono::high_resolution_clock::time_point startTime)
{
  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float, std::milli> timeDiff = now - startTime;
  insertWithTimestamp(timeDiff.count(), now);
}

float MyTimer::average()
{
  if (size() == 0)
  {
    return 0.0;
  }
  float sum = std::accumulate(buffer.begin(), buffer.end(), 0.0f);
  return sum / static_cast<float>(size());
}

void MyTimer::print()
{
  size_t current = head;
  size_t count = 0;
  while (count < size())
  {
    std::cout << buffer[current] << " ms ";
    current = (current + 1) % buffer.size();
    ++count;
  }
  std::cout << std::endl;
}

size_t MyTimer::size()
{
  if (isFull)
  {
    return buffer.size();
  }
  if (tail >= head)
  {
    return tail - head;
  }
  return buffer.size() - (head - tail);
}

void MyTimer::insertWithTimestamp(float value, std::chrono::high_resolution_clock::time_point now)
{
  buffer[tail] = value;
  tail = (tail + 1) % buffer.size();
  if (isFull)
  {
    head = (head + 1) % buffer.size();
  }
  isFull = tail == head;
  lastInsertTime = now;
  #ifdef DEBUG_MODE
  measurements.push_back(value);
  #endif
}

#ifdef DEBUG_MODE
void MyTimer::saveMeasurements(const std::string& fName)
{
  std::string fullFPath = BUILD_DIR + fName + ".txt";
  std::ofstream file(fullFPath);
  if (file.is_open())
  {
    for (float val: measurements)
    {
      file << val << " ";
    }
    file.close();
  }
}
#endif

const float& MyTimer::getLast()
{
  return buffer[tail];
}