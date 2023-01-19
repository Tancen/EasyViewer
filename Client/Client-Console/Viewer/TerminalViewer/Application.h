#ifndef APPLICATION_H
#define APPLICATION_H

#include <QCoreApplication>
#include <QTimer>
#include <QLockFile>
#include <memory>
#include "Command/TerminalViewer.h"

class Application : public QCoreApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application();

    bool init();


private:
    TerminalViewer* m_viewer;
};

#endif // APPLICATION_H
