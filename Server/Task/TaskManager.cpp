#include "TaskManager.h"
#include <atomic>
#include <functional>

#define MAX_TIMEOUT 57600000

Task::Manager* Task::Manager::s_this = nullptr;

Task::Manager *Task::Manager::share()
{
    return s_this;
}

bool Task::Manager::init(unsigned numWorker)
{
    assert(s_this == nullptr);
    s_this = new Manager(numWorker);
    return true;
}

void Task::Manager::release()
{
    delete s_this;
    s_this = nullptr;
}

Task::Manager::~Manager()
{
    exit();
    waitForExited();
}

void Task::Manager::exit()
{
    m_exit = true;
    {
        std::lock_guard g(m_mutex);
        m_cv.notify_all();
    }
}

void Task::Manager::waitForExited()
{
    for (auto& it : m_workers)
        if (it.joinable())
            it.join();
}

bool Task::Manager::dispatch(Task::ITaskPtr task, unsigned int msTimeout)
{
    if (msTimeout > MAX_TIMEOUT)
        msTimeout = MAX_TIMEOUT;

    return dispatch(task, QDateTime(), QDateTime::currentDateTime().addMSecs(msTimeout));
}

bool Task::Manager::dispatch(Task::ITaskPtr task, const QDateTime &executeAtTime)
{
    return dispatch(task, executeAtTime, executeAtTime);
}

bool Task::Manager::dispatch(Task::ITaskPtr task, const QDateTime &executeAtTime, unsigned int msTimeout)
{
    if (msTimeout > MAX_TIMEOUT)
        msTimeout = MAX_TIMEOUT;

    return dispatch(task, executeAtTime, executeAtTime.addMSecs(msTimeout));
}

bool Task::Manager::dispatch(Task::ITaskPtr task, const QDateTime &executeAtTime, QDateTime maxStayAtTime)
{
    bool ret = false;

    ItemPtr it(new Item{task, executeAtTime, maxStayAtTime, false});

    {
        std::lock_guard g(m_mutex);
        ret = m_mapItems.insert({task->id(), it}).second;
        if (ret)
        {
            pushHeap(m_heapItems, it);
            if (it->executeAtTime.toMSecsSinceEpoch() < m_heapItems.front()->executeAtTime.toMSecsSinceEpoch())
                m_cv.notify_one();
        }

    }
    return ret;
}

Task::ITaskPtr Task::Manager::destroy(Task::ID taskID, Task::Certificate certificate)
{
    Task::ITaskPtr ret;
    {
        std::lock_guard g(m_mutex);
        auto it = m_mapItems.find(taskID);
        if (it != m_mapItems.end())
        {
            if (it->second->task->certificate() == certificate)
            {
                ret = it->second->task;
            }
        }
    }

    if (ret.get())
    {
        ret->lock();
        bool b = ret->destroyed();
        if (!b)
            ret->destroy();
        ret->unlock();

        if (b)
            ret.reset();
    }

    return  ret;

}

Task::Manager::Manager(unsigned numWorker)
{
    for (unsigned i = 0; i < numWorker; i++)
    {
        m_workers.push_back(std::thread(std::bind(&Task::Manager::exec, this)));
    }
}

void Task::Manager::popHeap(std::vector<Task::Manager::ItemPtr> &heap)
{
    std::pop_heap(heap.begin(), heap.end(), compare);
    heap.pop_back();
}

void Task::Manager::pushHeap(std::vector<Task::Manager::ItemPtr> &heap, Task::Manager::ItemPtr &it)
{
    heap.push_back(it);
    std::push_heap(heap.begin(), heap.end(), compare);
}

void Task::Manager::exec()
{
    while (!m_exit)
    {
        QDateTime now = QDateTime::currentDateTime();

        do
        {
            m_mutex.lock();
            if (m_heapItems.empty())
            {
                m_mutex.unlock();
                break;
            }

            ItemPtr it = m_heapItems.front();

            if (it->task->destroyed())
            {
                popHeap(m_heapItems);
                m_mapItems.erase(it->task->id());
                m_mutex.unlock();
                break;
            }

            if (now > it->maxStayAtTime)
            {
                popHeap(m_heapItems);
                m_mapItems.erase(it->task->id());
                m_mutex.unlock();

                it->task->lock();
                if (!it->task->destroyed())
                {
                    it->task->timeout();
                    it->task->destroy();
                }
                it->task->unlock();
                break;
            }

            if (now > it->executeAtTime && !it->executed)
            {
                popHeap(m_heapItems);
                m_mutex.unlock();

                it->task->lock();
                if (!it->task->destroyed())
                {
                    it->executed = true;
                    it->task->execute();
                    it->task->unlock();

                    if (it->task->destroyed())
                    {
                        m_mutex.lock();
                        m_mapItems.erase(it->task->id());
                        m_mutex.unlock();
                    }
                    else
                    {
                        m_mutex.lock();
                        pushHeap(m_heapItems, it);
                        m_mutex.unlock();
                    }
                }
                else
                {
                    it->task->unlock();
                    m_mutex.lock();
                    m_mapItems.erase(it->task->id());
                    m_mutex.unlock();
                }
                break;
            }
            m_mutex.unlock();
        } while (0);

        {
            std::unique_lock lock(m_mutex);
            qint64 dif = 50;
            if (!m_heapItems.empty())
            {
                auto& it = m_heapItems.front();
                if (it->executed)
                    dif = it->maxStayAtTime.toMSecsSinceEpoch() - now.toMSecsSinceEpoch();
                else
                    dif = it->executeAtTime.toMSecsSinceEpoch() - now.toMSecsSinceEpoch();
            }

            if (dif > 0)
                m_cv.wait_for(lock, std::chrono::milliseconds(dif));
        }
    }
}
