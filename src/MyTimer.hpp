#ifndef MYTIMER_HPP
#define MYTIMER_HPP

#include <vector>
#include <chrono>
#include "globals.hpp"
#ifdef DEBUG_MODE
#include <fstream>
#endif

class MyTimer
{
  public:
    MyTimer(size_t size = TIMER_BUFF_SIZE);
    void insertTimeDifference();
    void insertTimeDifference(std::chrono::high_resolution_clock::time_point startTime);
    float average();
    void print();
    size_t size();
    #ifdef DEBUG_MODE
    void saveMeasurements(const std::string& fName);
    #endif
    const float& getLast();

  private:
    std::vector<float> buffer;
    size_t head, tail;
    bool isFull;
    std::chrono::high_resolution_clock::time_point lastInsertTime;
    void insertWithTimestamp(float value, std::chrono::high_resolution_clock::time_point now);
    #ifdef DEBUG_MODE
    std::vector<float> measurements;
    #endif
};

#endif // MYTIMER_HPP