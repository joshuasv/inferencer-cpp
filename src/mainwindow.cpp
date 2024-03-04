#include <iostream>
#include <QFile>
#include "ui_mainwindow.h"
#include "mainwindow.hpp"

MainWindow::MainWindow(std::string modelFPath, std::variant<std::string, int> source, QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  if (std::holds_alternative<int>(source))
  {
    webcamFrameGrabber = new Worker(std::get<int>(source)); 
  }
  else
  {
    webcamFrameGrabber = new Worker(std::get<std::string>(source)); 
  }
  inferencer = new ONNXInferencer(modelFPath);
  commonInit();
}

MainWindow::~MainWindow()
{
  delete webcamFrameGrabber;
  delete ui;
}

void MainWindow::commonInit(void)
{
  ui->setupUi(this);
  inferenceThread = new QThread();
  inferencer->moveToThread(inferenceThread);
  inferenceThread->start();

  connect(webcamFrameGrabber, &Worker::frameCaptured, inferencer, &ONNXInferencer::runInference, Qt::QueuedConnection);
  connect(webcamFrameGrabber, &Worker::frameCaptured, this, &MainWindow::updateDisplay);
  connect(inferencer, &ONNXInferencer::frameReady, this, &MainWindow::updateInferenceDisplay);
  connect(inferenceThread, &QThread::finished, inferencer, &QObject::deleteLater);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Q)
  {
    webcamFrameGrabber->terminate();
    // onnxInference->terminate();
    close();
  }
  else if (event->key() == Qt::Key_S)
  {
    webcamFrameGrabber->start(QThread::HighPriority);
    // onnxInference->start(QThread::HighPriority);
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

void MainWindow::updateInferenceDisplay(const cv::Mat &mat)
{
  QImage qImg = cvMatToQImage(mat);
  ui->inferenceLabel->setPixmap(
    QPixmap::fromImage(qImg).scaled(
      ui->inferenceLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation
  ));
}

void MainWindow::updateDisplay(const cv::Mat &mat)
{
  // Displays frame to window
  QImage qImg = cvMatToQImage(mat);
  ui->frameLabel->setPixmap(
    QPixmap::fromImage(qImg).scaled(
      ui->frameLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation
  ));
}