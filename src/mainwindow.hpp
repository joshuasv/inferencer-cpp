#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <variant>
#include <QMainWindow>
#include <QKeyEvent>
#include <QImage>
#include "Worker.hpp"
#include "ONNXInferencer.hpp"
#include "ObjectDetectionDrawer.hpp"
#include "Redis.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(std::string modelFPath, std::variant<std::string, int> source, std::string redisHost, int redisPort, QWidget* parent = nullptr);
        ~MainWindow();
        std::vector<int> imgSize = {640, 640};
        const int64_t tensorShape[4] = {1, 3, imgSize.at(0), imgSize.at(1)};
        const size_t shapeLen = sizeof(tensorShape) / sizeof(tensorShape[0]);

    private:
        Ui::MainWindow* ui;
        Worker* webcamFrameGrabber;
        ONNXInferencer* inferencer;
        ObjectDetectionDrawer* drawer;
        Redis* redis;
        QThread* inferencerThread;
        QThread* drawerThread;
        QThread* redisThread;
        void commonInit(void);
        QImage cvMatToQImage(const cv::Mat& mat);

    protected slots:
        void keyPressEvent(QKeyEvent* event) override;
        void updateDisplay(const cv::Mat& mat);
        void updateInferenceDisplay(const cv::Mat& mat);
        void updateTimerFrameLabel(const float& value);
        void updateTimerInferencerLabel(const float& value);
        void updateTimerDrawerLabel(const float& value);
        void updateTimerRedisLabel(const float& value);
};

#endif // MAINWINDOW_HPP