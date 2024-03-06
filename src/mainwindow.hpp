#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <variant>
#include <QMainWindow>
#include <QKeyEvent>
#include <QImage>
#include "Worker.hpp"
#include "ONNXInferencer.hpp"
#include "ObjectDetectionDrawer.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(std::string modelFPath, std::variant<std::string, int> source, QWidget* parent = nullptr);
        ~MainWindow();
        std::vector<int> imgSize = {640, 640};
        const int64_t tensorShape[4] = {1, 3, imgSize.at(0), imgSize.at(1)};
        const size_t shapeLen = sizeof(tensorShape) / sizeof(tensorShape[0]);

    private:
        Ui::MainWindow* ui;
        Worker* webcamFrameGrabber;
        ONNXInferencer* inferencer;
        ObjectDetectionDrawer* drawer;
        QThread* inferencerThread;
        QThread* drawerThread;
        void commonInit(void);
        QImage cvMatToQImage(const cv::Mat& mat);

    protected slots:
        void keyPressEvent(QKeyEvent* event) override;
        void updateDisplay(const cv::Mat& mat);
        void updateInferenceDisplay(const cv::Mat& mat);
};

#endif // MAINWINDOW_HPP