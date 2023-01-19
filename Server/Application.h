#ifndef APPLICATION_H
#define APPLICATION_H

#include <QCoreApplication>
#include <QTimer>
#include <QLockFile>
#include <memory>

class Application : public QCoreApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application();

    bool init();

private slots:
    void dealWithTriviality();

private:
    QTimer m_timer;
    QTimer m_timerClearLog;
    unsigned m_logDays;
    std::unique_ptr<QLockFile> m_lockFile;
};

#endif // APPLICATION_H
