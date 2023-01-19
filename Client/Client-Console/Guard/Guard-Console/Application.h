#ifndef APPLICATION_H
#define APPLICATION_H

#include <QCoreApplication>
#include <QTimer>
#include <QLockFile>
#include <memory>
#include "Guard.h"

class Application : public QCoreApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application();

    bool init();

private slots:
    void connectionCheck();

private:
    std::unique_ptr<Guard> m_guard;
    QTimer m_timer;
    QTimer m_timerClearLog;;
};

#endif // APPLICATION_H
