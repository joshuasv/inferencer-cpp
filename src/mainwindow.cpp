#include <iomanip>
#include <iostream>
#include <QFile>
#include "ui_mainwindow.h"
#include "mainwindow.hpp"
#include "globals.hpp"

MainWindow::MainWindow(std::string modelFPath, std::variant<std::string, int> source, std::string redisHost, int redisPort, QWidget* parent)
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
  // TODO: refactor
  const std::vector<std::string> classes{"white", "blue", "orange"};
  const std::vector<cv::Scalar> colors{cv::Scalar(255, 255, 255), cv::Scalar(165, 0, 0), cv::Scalar(0, 165, 255)};
  redis = new Redis(redisHost, redisPort);
  drawer = new ObjectDetectionDrawer(classes, colors, redis, "order_updates");
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
  inferencerThread = new QThread();
  drawerThread = new QThread();
  redisThread = new QThread();
  inferencer->moveToThread(inferencerThread);
  drawer->moveToThread(drawerThread);
  redis->moveToThread(redisThread);
  

  connect(webcamFrameGrabber, &Worker::frameCaptured, inferencer, &ONNXInferencer::runInference, Qt::QueuedConnection);
  connect(webcamFrameGrabber, &Worker::frameCaptured, this, &MainWindow::updateDisplay);
  connect(webcamFrameGrabber, &Worker::updateTimer, this, &MainWindow::updateTimerFrameLabel);
  connect(inferencer, &ONNXInferencer::resultsReady, drawer, &ObjectDetectionDrawer::draw, Qt::QueuedConnection);
  connect(inferencer, &ONNXInferencer::sendResults, redis, &Redis::sendDict, Qt::QueuedConnection);
  connect(inferencer, &ONNXInferencer::updateTimer, this, &MainWindow::updateTimerInferencerLabel);
  connect(drawer, &ObjectDetectionDrawer::frameReady, this, &MainWindow::updateInferenceDisplay);
  connect(drawer, &ObjectDetectionDrawer::updateTimer, this, &MainWindow::updateTimerDrawerLabel);
  connect(redis, &Redis::updateTimer, this, &MainWindow::updateTimerRedisLabel);
  connect(inferencerThread, &QThread::finished, inferencer, &QObject::deleteLater);
  connect(drawerThread, &QThread::finished, drawer, &QObject::deleteLater);
  connect(redisThread, &QThread::finished, redis, &QObject::deleteLater);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Q)
  {
    IS_RUNNING = false;
    webcamFrameGrabber->quit();
    inferencerThread->quit();
    drawerThread->quit();
    redisThread->quit();
    webcamFrameGrabber->wait();
    inferencerThread->wait();
    drawerThread->wait();
    redisThread->wait();
    close();
  }
  else if (event->key() == Qt::Key_S)
  {
    IS_RUNNING = true;
    webcamFrameGrabber->start(QThread::HighPriority);
    inferencerThread->start(QThread::HighPriority);
    drawerThread->start(QThread::HighPriority);
    redisThread->start(QThread::HighPriority);
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

void MainWindow::updateTimerFrameLabel(const float& value)
{
  std::string msg = "Frame grabber: ";
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(2) << value;
  std::string formattedValue = stream.str();
  std::string result = msg + formattedValue;

  ui->timerFrameLabel->setText(result.c_str());
}

void MainWindow::updateTimerInferencerLabel(const float& value)
{
  std::string msg = "Inferencer: ";
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(2) << value;
  std::string formattedValue = stream.str();
  std::string result = msg + formattedValue;

  ui->timerInferencerLabel->setText(result.c_str());
}
void MainWindow::updateTimerDrawerLabel(const float& value)
{
  std::string msg = "Drawer: ";
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(2) << value;
  std::string formattedValue = stream.str();
  std::string result = msg + formattedValue;

  ui->timerDrawerLabel->setText(result.c_str());
}

void MainWindow::updateTimerRedisLabel(const float& value)
{
  std::string msg = "Redis: ";
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(2) << value;
  std::string formattedValue = stream.str();
  std::string result = msg + formattedValue;

  ui->timerRedisLabel->setText(result.c_str());
}