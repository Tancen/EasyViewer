#ifdef __linux__
#ifndef EASYIOTCPCONNECTION_H
#define EASYIOTCPCONNECTION_H

#include <string>
#include <functional>
#include "EasyIODef.h"
#include "EasyIOByteBuffer.h"
#include "EasyIOContext_linux.h"
#include "EasyIOEventLoop_linux.h"
#include "EasyIOTCPIConnection.h"
#include <list>
#include <mutex>
#include <atomic>

namespace EasyIO
{
    namespace TCP
    {
        class Connection : virtual public IConnection
        {
        public:
            Connection(EventLoop *worker);
            Connection(EventLoop *worker, SOCKET sock, bool connected = true);
            ~Connection();

            IConnectionPtr share() override;
            SOCKET handle() override;

            bool connected() override;

            void disconnect() override;
            void syncDisconnect() override;

            void send(ByteBuffer buffer) override;
            void recv(ByteBuffer buffer) override;
            int numBytesPending() override;

            bool enableKeepalive(unsigned long interval = 1000, unsigned long time = 2000) override;
            bool disableKeepalive() override;

            bool setSendBufferSize(unsigned long size) override;
            bool setReceiveBufferSize(unsigned long size) override;

            bool setLinger(unsigned short onoff, unsigned short linger) override;

            const std::string& localIP() const  override;
            unsigned short localPort() const  override;

            const std::string& peerIP() const  override;
            unsigned short peerPort() const  override;

            bool updateEndPoint() override;

            void bindUserdata(void* userdata) override;
            void* userdata() const  override;

        protected:
            int send0();
            int recv0();
            void disconnect(const std::string& reason);
            void disconnect0(bool requireUnlock);

            bool sendComplete(ByteBuffer buffer);
            bool recvComplete(ByteBuffer buffer);

            bool addTask(ByteBuffer buffer, std::list<ByteBuffer>& dst);
            int doFirstTask(std::list<ByteBuffer>& tasks, std::function<int(ByteBuffer)> transmitter,
                             std::function<bool(ByteBuffer)> isComplete,
                             std::function<void (IConnection*, ByteBuffer)> onComplete);
            void popFirstTask(std::list<ByteBuffer>& tasks);
            void cleanTasks(std::list<ByteBuffer>& tasks);

            void handleEvents(uint32_t events);

            virtual IConnectionPtr makeHolder();

        protected:
            SOCKET m_handle;
            EventLoop* m_worker;
            Context::Context m_context;

            bool m_connected;
            bool m_disconnecting;

            std::string m_localIP;
            unsigned short m_localPort;

            std::string m_peerIP;
            unsigned short m_peerPort;

            std::recursive_mutex m_lock;

            std::list<ByteBuffer> m_tasksSend;
            std::list<ByteBuffer> m_tasksRecv;

            void* m_userdata;

            std::atomic<int> m_numBytesPending;
            std::string m_reason;
        };
    }
}

#endif
#endif
