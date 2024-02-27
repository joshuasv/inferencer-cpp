#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QKeyEvent>
#include <QImage>
#include "Worker.hpp"
#include "Inferencer.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private:
        Ui::MainWindow *ui;
        Worker *webcamFrameGrabber;
        Inferencer *inferencer;
        QImage cvMatToQImage(const cv::Mat &mat);
        void updateDisplay(const cv::Mat &mat);

    protected slots:
        void AtEnd(void);
        void Feedback(const int &n);
        void TimeOut(void);
        void keyPressEvent(QKeyEvent *event) override;

};

#endif // MAINWINDOW_HPP