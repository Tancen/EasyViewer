#ifdef __linux__
#ifndef EASYIOUDPSOCKET_LINUX_H
#define EASYIOUDPSOCKET_LINUX_H

#include "EasyIOUDPISocket.h"
#include "EasyIOContext_linux.h"
#include "EasyIOEventLoop_linux.h"
#include <functional>
#include <atomic>
#include <mutex>
#include <list>

namespace EasyIO
{
    namespace UDP
    {
        class Socket : virtual public ISocket
        {
            class Task;
            typedef std::shared_ptr<Task> TaskPtr;
        public:
            Socket(EventLoop *worker);
            Socket(EventLoop *worker, SOCKET sock);
            ~Socket();

            ISocketPtr share() override;
            SOCKET handle() override;

            bool opened() override;

            void close() override;
            void send(const std::string& ip, unsigned short port, ByteBuffer buffer) override;
            void recv(ByteBuffer buffer) override;
            size_t numBytesPending() override;

            const std::string& localIP() const  override;
            unsigned short localPort() const  override;

            bool updateEndPoint() override;

            void bindUserdata(void* userdata) override;
            void* userdata() const  override;

        protected:
            int send0();
            int recv0();
            void close0();

            bool sendComplete(TaskPtr buffer);
            bool recvComplete(TaskPtr buffer);

            bool addTask(TaskPtr task, std::list<TaskPtr>& dst);
            int doFirstTask(std::list<TaskPtr>& tasks, std::function<int(TaskPtr)> transmitter,
                             std::function<bool(TaskPtr)> isComplete,
                             std::function<void (ISocket*, const std::string&, unsigned short, ByteBuffer)> onComplete);
            void popFirstTask(std::list<TaskPtr>& tasks);
            void cleanTasks(std::list<TaskPtr>& tasks);

            void handleEvents(uint32_t events);

        protected:
            SOCKET m_handle;
            EventLoop* m_worker;
            Context::Context m_context;

            bool m_opened;
            bool m_closing;

            std::string m_localIP;
            unsigned short m_localPort;

            std::recursive_mutex m_lock;

            std::list<TaskPtr> m_tasksSend;
            std::list<TaskPtr> m_tasksRecv;

            void* m_userdata;

            std::atomic<size_t> m_numBytesPending;
        };
    }
}

#endif
#endif
