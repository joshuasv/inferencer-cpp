#include <iostream>
#include "ui_mainwindow.h"
#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  webcamFrameGrabber = new Worker(); 
  inferencer = new Inferencer();

  connect(webcamFrameGrabber, &Worker::frameCaptured, inferencer, &Inferencer::doInference);
  connect(inferencer, &Inferencer::frameProcessed, this, &MainWindow::updateDisplay);

  // connect(webcamFrameGrabber, &Worker::frameCaptured, this, &MainWindow::updateDisplay);
}

MainWindow::~MainWindow()
{
  delete webcamFrameGrabber;
  delete ui;
}

void MainWindow::TimeOut(void)
{
  std::cout << "MainWindow::TimeOut enter\n";
}

void MainWindow::Feedback(const int &n)
{
  std::cout << "MainWindow::Feedback enter\n";
}

void MainWindow::AtEnd(void)
{
  std::cout << "MainWindow::AtEnd enter\n";
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Q)
  {
    webcamFrameGrabber->terminate();
    inferencer->terminate();
    close();
  }
  else if (event->key() == Qt::Key_S)
  {
    webcamFrameGrabber->start(QThread::HighPriority);
    inferencer->start(QThread::HighPriority);
  }
  else
  {
    QMainWindow::keyPressEvent(event);
  }
}

QImage MainWindow::cvMatToQImage(const cv::Mat &mat)
{
  switch (mat.type()) {
  case CV_8UC4: {
    QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
    return image.copy();
  }
  case CV_8UC3: {
    QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
    return image.rgbSwapped();
  }
  case CV_8UC1: {
    QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
    return image.copy();
  }
  default:
    qWarning("Unsupported format for conversion to QImage");
    break;
  }
  return QImage();
}

void MainWindow::updateDisplay(const cv::Mat &frame)
{
  std::cout << "MainWindow::updateDisplay\n";
  QImage qImg = cvMatToQImage(frame);
  ui->imageLabel->setPixmap(
    QPixmap::fromImage(qImg).scaled(
      ui->imageLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation
  ));
}