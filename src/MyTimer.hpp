#ifndef MYTIMER_HPP
#define MYTIMER_HPP

#include <vector>
#include <chrono>

class MyTimer
{
  public:
    MyTimer(size_t size);
    void insertTimeDifference();
    float average();
    void print();
    size_t size();

  private:
    std::vector<float> buffer;
    size_t head, tail;
    bool isFull;
    std::chrono::high_resolution_clock::time_point lastInsertTime;
    void insertWithTimestamp(float value, std::chrono::high_resolution_clock::time_point now);
};

#endif // MYTIMER_HPP