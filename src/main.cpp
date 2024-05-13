#include <variant>
#include <QFile>
#include <QApplication>
#include "mainwindow.hpp"

bool checkArgs(QStringList& args)
{
  // Check the right number of arguments were provided 
  if (args.size() < 5)
  {
    qCritical() << "Usage:" << args.at(0) << "MODEL_FPATH CAM_ID REDIS_HOST REDIS_PORT";
    qCritical() << "SOURCE can be a camera index (int) or a video file path.";
    return false;
  }

  QString modelFPath = args.at(1);
  QString source = args.at(2);
  QString redisHost = args.at(3);
  QString redisPort = args.at(4);

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

  // Check Redis port is int
  int redisPortTmp = redisPort.toInt(&isInt);
  if (!isInt)
  {
    qCritical() << "The specified Redis port is not an int" << redisPort;
    return false;
  }

  return true;
}

int main(int argc, char *argv[])
{
  #ifdef DEBUG_MODE
    std::cout << "DEBUG_MODE: ON\n";
  # endif

  QApplication a(argc, argv);
  QStringList args = a.arguments();
  bool argsOk = checkArgs(args);
  if (!argsOk)
  {
    return -1;
  }
  std::string modelFPath = args.at(1).toStdString();
  std::variant<std::string, int> source;
  std::string redisHost = args.at(3).toStdString();
  int redisPort = args.at(4).toInt();

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
  MainWindow w(modelFPath, source, redisHost, redisPort);
  w.show();

  return a.exec();
}