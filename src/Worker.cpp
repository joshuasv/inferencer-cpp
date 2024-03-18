#include "Worker.hpp"

// static uchar pixels[W*H*D];

Worker::Worker(int source, QObject *parent): QThread(parent)
{
  cap.open(source, cv::CAP_V4L);
  if (!cap.isOpened())
  {
    std::cerr << "Unable to open camera: " << source << "\n";
  }
  sourceFPS = cap.get(cv::CAP_PROP_FPS);
  std::cout << "FPS: " << cap.get(cv::CAP_PROP_FPS) << std::endl;
  std::cout << "H: " << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;
  std::cout << "W: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << std::endl;
}

Worker::Worker(std::string source, QObject *parent): QThread(parent)
{
  cap.open(source); 
  if (!cap.isOpened())
  {
    std::cerr << "Unable to open video file: " << source << "\n";
  }
  sourceFPS = cap.get(cv::CAP_PROP_FPS);
  isFile = true;
  frameDelay = int(1000 / sourceFPS);
}

Worker::~Worker()
{
  if (cap.isOpened())
  {
    cap.release();
  }
  #ifdef DEBUG_MODE
  // Save timer measurements to file
  t.saveMeasurements("grabber_times");
  #endif
}

void Worker::run()
{
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
    
    t.insertTimeDifference();
    emit updateTimer(t.average());
    
    if (isFile)
    {
      QThread::msleep(frameDelay);
    }
  }
}