#ifdef __linux__
#ifndef EASYIOEVENTLOOP_H
#define EASYIOEVENTLOOP_H

#include <thread>
#include <sys/epoll.h>
#include <functional>
#include <mutex>
#include <vector>
#include "EasyIODef.h"
#include "EasyIOIEventLoop.h"
#include "EasyIOContext_linux.h"

namespace EasyIO
{
    class EventLoop : public IEventLoop
    {
        struct Task
        {
            std::function<void(void*)> callback;
            void* userdata;
        };
    public:
        static IEventLoopPtr create(int maxEvents = 1024);
        ~EventLoop();

        IEventLoopPtr share() override;

        void add(int fd, Context::Context *context);
        void modify(int fd, Context::Context *context);
        void remove(int fd, Context::Context *context);

    private:
        EventLoop();
        void execute(int maxEvents);

    private:
        int m_handle;
        bool m_exit;
        std::thread m_thread;
        __pid_t m_pid;
    };

}
#endif
#endif
