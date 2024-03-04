#include "Worker.hpp"

// static uchar pixels[W*H*D];

Worker::Worker(int source, QObject *parent): QThread(parent)
{
  cap.open(source, cv::CAP_V4L);
  if (!cap.isOpened())
  {
    std::cerr << "Unable to open camera: " << source << "\n";
  }
}

Worker::Worker(std::string source, QObject *parent): QThread(parent)
{
  cap.open(source); 
  if (!cap.isOpened())
  {
    std::cerr << "Unable to open video file: " << source << "\n";
  }
}

Worker::~Worker()
{
  if (cap.isOpened())
  {
    cap.release();
  }
}

void Worker::run()
{
  int frameDelay = int(1000/30);
  cv::Mat frame;
  while (cap.isOpened())  
  {
    cap >> frame;
    if (frame.empty())
    {
      std::cerr << "Blank frame\n";
      break;
    }
    emit frameCaptured(frame);
    // Simualte that we are grabbing at n FPS; TODO: remove
    QThread::msleep(frameDelay);
  }
}