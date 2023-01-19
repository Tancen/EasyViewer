#include "RequestProcessor.h"
#include <algorithm>
#include "Global/Component/Logger/Logger.h"

#define FLAG_ACTIVE_OFFSET  (sizeof(UserConnection::TAG_TYPE) * 8 - 1)

inline bool compare(const UserConnectionPtr& o1, const UserConnectionPtr& o2)
{
    long long dif;
    dif = (long long)o1->reservation - (long long)o2->reservation;
    if (dif == 0)
    {
        return o1->limit > o2->limit;
    }

    return dif > 0;
}

inline long long now()
{
    return QDateTime::currentMSecsSinceEpoch();
}

RequestProcessor RequestProcessor::s_this;
RequestProcessor *RequestProcessor::share()
{
    return &s_this;
}

void RequestProcessor::exit()
{
    for (auto& it : m_heaps)
    {
        it.second->exit();
    }
}

void RequestProcessor::waitForExited()
{
    for (auto& it : m_heaps)
    {
        it.second->waitForExited();
    }
}

bool RequestProcessor::push(UserConnectionPtr con, uint32_t tag, EasyIO::ByteBuffer data, NetworkDataHandler::INetworkDataHandlerPtr handler)
{
    UserConnection::Request r(con, now(), tag, data, handler);
    return push(r);
}

bool RequestProcessor::push(UserConnection::Request &r)
{
    Heap *heap = nullptr;
    {
        std::shared_lock g(m_mutex);
        auto it = m_heaps.find(std::this_thread::get_id());
        if (it != m_heaps.end())
            heap = it->second.get();
    }
    if (!heap)
    {
        std::unique_lock g(m_mutex);
        heap = new Heap();
        m_heaps.insert({std::this_thread::get_id(), std::unique_ptr<Heap>(heap)});
    }
    return heap->push(r);
}

RequestProcessor::RequestProcessor()
{

}

RequestProcessor::Heap::Heap()
{
    m_thread = std::thread(std::bind(&RequestProcessor::Heap::exec, this));
}

RequestProcessor::Heap::~Heap()
{
    exit();
    waitForExited();
}

void RequestProcessor::Heap::exit()
{
    m_exit = true;
    m_cv.notify_all();
}

void RequestProcessor::Heap::waitForExited()
{
    if (m_thread.joinable())
        m_thread.join();
}

bool RequestProcessor::Heap::push(UserConnection::Request &r)
{
    long long t = now();
    std::unique_lock g(m_mutex);
    bool emptied = m_queue.empty();
    auto pair = r.con->pushRequest(r);
    if (!pair.first)
        return false;

    if (pair.second == 0)
    {
        if (r.con->reservation <= t)
        {
            UserConnection::Request r1;
            auto success = r.con->popRequest(r1);
            assert(success);
            r1.handler->handle(r1.con, r1.data.uReadableBytes(), r1.data.numReadableBytes());
            return true;
        }
        else
        {
            m_queue.push_back(r.con);
            std::push_heap(m_queue.begin(), m_queue.end(), compare);
        }
    }
    size_t n = processPossible();
    if (emptied && n)
        m_cv.notify_all();

    return true;
}

std::pair<bool, size_t> RequestProcessor::Heap::pull(long long t, UserConnection::Request &r)
{
    if (m_queue.empty())
        return {false, 0};

    UserConnectionPtr client = nullptr, c = nullptr;

    //phase 1
    c = m_queue[0];
    if (c->reservation <= t)
    {
        client =  c;
        std::pop_heap(m_queue.begin(), m_queue.end(), compare);
        m_queue.pop_back();
    }
    //phase 2
    else
    {
        int index = 0;
        static const int MAX_COUNT = 20;

        for (int i = 0; i < m_queue.size() && i < MAX_COUNT; i++)
        {
            UserConnectionPtr it = m_queue[i];
            if (it->limit <= t)
            {
                if (!client || it->proportion <= client->proportion)
                {
                    client = it;
                    index = i;
                }
            }
            else
                break;
        }
        if (client)
        {
            auto r = client->reservation;
            auto l = client->limit;
            auto p = client->proportion;

            // pop
            client->reservation = std::numeric_limits<double>::min();
            client->limit = std::numeric_limits<double>::min();
            std::push_heap(m_queue.begin(), m_queue.begin() + index + 1, compare);
            std::pop_heap(m_queue.begin(), m_queue.end(), compare);
            m_queue.pop_back();

            client->reservation = r;
            client->limit = l;
            client->proportion = p;
        }
    }

    if (client)
    {
        bool success = client->popRequest(r);
        assert(success);
        if (!client->isRequestsEmpty())
        {
            m_queue.push_back(client);
            std::push_heap(m_queue.begin(), m_queue.end(), compare);
        }
        return {true, m_queue.size()};
    }

    return {false, m_queue.size()};
}

size_t RequestProcessor::Heap::processPossible()
{
    long long t = now();
    do
    {
        UserConnection::Request r;
        auto pair = pull(t, r);
        if (!pair.first)
            return pair.second;

        r.handler->handle(r.con, r.data.uReadableBytes(), r.data.numReadableBytes());
    } while (1);
}

void RequestProcessor::Heap::exec()
{
    while (!m_exit)
    {
        {
            std::unique_lock g(m_mutex);
            auto n = processPossible();
            if (!n)
                m_cv.wait(g);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
