#if  defined(WIN32) || defined(WIN64)
#ifndef EASYIOTCPCONNECTION_H
#define EASYIOTCPCONNECTION_H

#include <string>
#include <functional>
#include "EasyIOByteBuffer.h"
#include "EasyIOContext_win.h"
#include <set>
#include <mutex>
#include <atomic>
#include "EasyIOTCPIConnection.h"

namespace EasyIO
{
    namespace TCP
    {
        class Connection : virtual public IConnection
        {
        public:
            Connection();
            Connection(SOCKET sock, bool connected = true);
            ~Connection();

            IConnectionPtr share() override;

            SOCKET handle() override;

            bool connected() override;

            void disconnect() override;
            void syncDisconnect() override;
            bool disconnecting();

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
            void disconnect0(bool requireDecrease, bool requireUnlock, const std::string& reason);

            bool addTask(Context *ctx, std::list<Context*>& dst);
            int doFirstTask(std::list<Context*>& tasks, std::function<int(Context*)> transmitter);
            int popFirstTask(std::list<Context*>& tasks);
            void cleanTasks(std::list<Context*>& tasks);

            void whenSendDone(Context *ctx, size_t increase);
            void whenRecvDone(Context *ctx, size_t increase);
            void whenError(Context *ctx, int err);

            int increasePostCount();
            int decreasePostCount();

            virtual IConnectionPtr makeHolder();

        protected:
            SOCKET m_handle;

            bool m_connected;
            bool m_disconnecting;
            std::atomic<int> m_numBytesPending;

            std::string m_localIP;
            unsigned short m_localPort;

            std::string m_peerIP;
            unsigned short m_peerPort;

            std::recursive_mutex m_lock;
            void* m_userdata;

            std::atomic<int> m_countPost;
            std::list<Context*> m_tasksSend;
            std::list<Context*> m_tasksRecv;
        };
    }

}

#endif
#endif
