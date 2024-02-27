#include "Worker.hpp"

// static uchar pixels[W*H*D];

Worker::Worker(QObject *parent): QThread(parent)
{
  int cameraIndex = 2;
  int cameraAPI = cv::CAP_V4L;
  cap.open(cameraIndex, cameraAPI);
  if (!cap.isOpened())
  {
    std::cerr << "Unable to open camera: " << cameraIndex << "\n";
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
  }
}

void Worker::work()
{

}