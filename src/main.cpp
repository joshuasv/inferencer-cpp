#include <variant>
#include <QFile>
#include <QApplication>
#include "mainwindow.hpp"

bool checkArgs(QStringList& args)
{
  // Check the right number of arguments were provided 
  if (args.size() < 3)
  {
    qCritical() << "Usage:" << args.at(0) << "MODEL_PATH SOURCE";
    qCritical() << "SOURCE can be a camera index (int) or a video file path.";
    return false;
  }

  QString modelFPath = args.at(1);
  QString source = args.at(2);

  // Check model file path exist
  if (!QFile::exists(modelFPath))
  {
    qCritical() << "The specified model path does not exist:" << modelFPath;
    return false;
  }
  
  // If stream source is not int, check if file exist
  bool isInt;
  int cameraIndex = source.toInt(&isInt);
  if (!isInt && !QFile::exists(source))
  {
    qCritical() << "The specified source file does not exist:" << source;
    return false;
  }

  return true;
}

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  QStringList args = a.arguments();
  bool argsOk = checkArgs(args);
  if (!argsOk)
  {
    return -1;
  }
  std::string modelFPath = args.at(1).toStdString();
  std::variant<std::string, int> source;
  bool isInt;
  int sourceInt = args.at(2).toInt(&isInt);
  if (isInt)
  {
    source = sourceInt;
  }
  else
  {
    source = args.at(2).toStdString();
  }
  MainWindow w(modelFPath, source);
  w.show();

  return a.exec();
}