#if  defined(WIN32) || defined(WIN64)
#ifndef EASYIOUDPSOCKET_H
#define EASYIOUDPSOCKET_H

#include "EasyIOUDPISocket.h"
#include "EasyIOContext_win.h"
#include <functional>
#include <atomic>
#include <mutex>

namespace EasyIO
{
    namespace UDP
    {
        class Socket : virtual public ISocket
        {
            class Context1 : public ::EasyIO::Context
            {
            public:
                Context1(ByteBuffer buffer, Flag flag);

                sockaddr_in* addr();
                int* addrLen();

            private:
                sockaddr_in m_addr;
                int m_addrLen;
            };

        public:
            Socket();
            Socket(SOCKET sock);
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
            void close0(bool requireDecrease);

            bool addTask(Context1 *ctx, std::list<Context1*>& dst);
            int doFirstTask(std::list<Context1*>& tasks, std::function<int(Context1*)> transmitter);
            void popFirstTask(std::list<Context1*>& tasks);
            void cleanTasks(std::list<Context1*>& tasks);

            void whenSendDone(Context *ctx, size_t increase);
            void whenRecvDone(Context *ctx, size_t increase);
            void whenError(Context *ctx, int err);

            int increasePostCount();
            int decreasePostCount();

        protected:
            SOCKET m_handle;

            bool m_opened;
            bool m_closing;
            std::atomic<size_t> m_numBytesPending;

            std::string m_localIP;
            unsigned short m_localPort;

            std::recursive_mutex m_lock;
            void* m_userdata;

            std::atomic<int> m_countPost;
            std::list<Context1*> m_tasksSend;
            std::list<Context1*> m_tasksRecv;
        };
    }

}


#endif
#endif
