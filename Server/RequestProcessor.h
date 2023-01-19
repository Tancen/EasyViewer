#ifndef REQUESTPROCESSOR_H
#define REQUESTPROCESSOR_H

#include "Global/Component/EasyIO/EasyIO.h"
#include "UserConnection.h"
#include <unordered_map>
#include <shared_mutex>

class RequestProcessor
{
    static const int CLIENT_MAP_SIZE = 512;

    class Heap
    {
    public:
        Heap();
        ~Heap();

        void exit();
        void waitForExited();

        bool push(UserConnection::Request& r);

    private:
        std::pair<bool, size_t> pull(long long t, UserConnection::Request& r);
        size_t processPossible();

        void exec();
    private:
        bool m_exit = false;
        std::vector<UserConnectionPtr> m_queue;
        std::thread m_thread;
        std::recursive_mutex m_mutex;
        std::condition_variable_any m_cv;
    };


public:
    static RequestProcessor* share();
    void exit();
    void waitForExited();

    bool push(UserConnectionPtr con, uint32_t tag, EasyIO::ByteBuffer data, NetworkDataHandler::INetworkDataHandlerPtr handler);
    bool push(UserConnection::Request& r);

private:
    RequestProcessor();

private:
    std::unordered_map<std::thread::id, std::unique_ptr<Heap>> m_heaps;

    std::shared_mutex m_mutex;
    static RequestProcessor s_this;
};

#endif // REQUESTPROCESSOR_H
