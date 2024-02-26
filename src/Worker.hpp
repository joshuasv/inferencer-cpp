#ifndef WORKER_HPP
#define WORKER_HPP

#include <random>
#include <QThread>
#include "globals.hpp"

class Worker: public QThread
{
  Q_OBJECT
  
  public:
    Worker(QObject *parent = nullptr);
    ~Worker();

  private:
    void run(void);
    void work(void);

  signals:
    void result(const int &N);
    void timeOut(void);
};

#endif  // WORKER_HPP