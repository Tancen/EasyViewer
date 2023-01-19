#include <QtGlobal>


#ifndef TERMINALMANAGER_H
#define TERMINALMANAGER_H

#include "Global/Define.h"
#include <string>
#include <functional>
#include <map>
#include <thread>
#ifdef Q_OS_WIN
    #include <Winsock2.h>
#else
    #include <sys/epoll.h>
    #include <unistd.h>
    #include <signal.h>
    #include <sys/wait.h>
    #include <sys/types.h>
#endif
#include <mutex>
#include "Global/Component/Logger/Logger.h"

class TerminalManager
{
#ifdef Q_OS_WIN
    struct Item : public OVERLAPPED
    {
    public:
        Item()
        {
            memset(&pi, 0, sizeof(pi));
        }

        ~Item()
        {
            CloseHandle(pi.hProcess);

            CloseHandle(outPipeOurSide);
            CloseHandle(inPipeOurSide);
            CloseHandle(outPipePseudoConsoleSide);
            CloseHandle(inPipePseudoConsoleSide);

            ClosePseudoConsole(hPC);

            Logger::info("terminal %s, pid %d destroyed", id.c_str(), pi.dwProcessId);
        }

        void inc()
        {
            ++m_ref;
        }

        void dec()
        {
            int v = --m_ref;
            if (v == 0)
                delete this;
        }

        std::string id;
        User::ID userId = 0;
        HPCON hPC = 0;
        PROCESS_INFORMATION pi;
        HANDLE outPipeOurSide = nullptr, inPipeOurSide = nullptr;
        HANDLE outPipePseudoConsoleSide = nullptr, inPipePseudoConsoleSide = nullptr;

    private:
        std::atomic<int> m_ref = 1;
    };
#else
    struct Item : public epoll_event
    {
    public:
        ~Item()
        {
            close(mpty);

            kill(pid, SIGKILL);
            int status;
            do
            {
                auto ret = waitpid(pid, &status, 0);
                if (ret == -1)
                    break;

                if (WIFEXITED(status))
                    break;

            } while (1);

            Logger::info("terminal %s, pid %d destroyed", id.c_str(), pid);
        }

        void inc()
        {
            ++m_ref;
        }

        void dec()
        {
            int v = --m_ref;
            if (v == 0)
                delete this;
        }

        std::string id;
        User::ID userId = 0;
        int mpty = 0;
        pid_t pid = 0;


    private:
        std::atomic<int> m_ref = 1;
    };
    typedef int _HANDLE;
#endif

public:
    ~TerminalManager();

    static TerminalManager* create();

    void exit();
    void waitForExited();

    std::string createTerminal(User::ID userId, short width, short height, bool *isOk);
    void writeCommand(const std::string& terminalId, const char *str, size_t len);
    void closeTerminal(const std::string& terminalId);
    void resizeTerminal(const std::string& terminalId, short width, short height);

private:
     TerminalManager();

    void exec();

public:
    std::function<void(User::ID userId, const std::string& terminalId,
                const char* str, size_t len)> onTerminalOutputted;
    std::function<void(User::ID userId, const std::string& terminalId)> onTerminalClosed;

private:
    std::map<std::string, Item*> m_items;
#ifndef Q_OS_WIN
    _HANDLE m_hEventLoop;
#endif
    std::recursive_mutex m_mutex;
    std::thread m_thread;
    bool m_exit;
};

#endif // TERMINALMANAGER_H
