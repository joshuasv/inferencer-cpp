#include <iostream>
#include "ui_mainwindow.h"
#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  webcamFrameGrabber = new Worker(); 
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
    close();
  }
  else
  {
    QMainWindow::keyPressEvent(event);
  }
}