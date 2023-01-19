#if  defined(WIN32) || defined(WIN64)
#ifndef EASYIOEVENTLOOP_H
#define EASYIOEVENTLOOP_H

#include <thread>
#include "EasyIODef.h"
#include "EasyIOIEventLoop.h"

namespace EasyIO
{
    class EventLoop : public IEventLoop
    {
    public:
        static IEventLoopPtr create();
        ~EventLoop();

        IEventLoopPtr share() override;

        bool attach(SOCKET sock, int& err);
        void detach(SOCKET sock);

    private:
        EventLoop();
        void execute();

    private:
        void* m_handle;
        bool m_exit;
        std::thread m_thread;
    };
}

#endif
#endif
