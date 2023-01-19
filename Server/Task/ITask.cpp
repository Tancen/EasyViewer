#include "ITask.h"

Task::ITask::ITask(Task::Certificate certificate)
    :   m_id(generateId()), m_certificate(certificate)
{

}

Task::ID Task::ITask::id()
{
    return m_id;
}

Task::Certificate Task::ITask::certificate()
{
    return m_certificate;
}

void Task::ITask::lock()
{
    m_mutex.lock();
}

void Task::ITask::unlock()
{
    m_mutex.unlock();
}

void Task::ITask::destroy()
{
    m_destroyed = true;
}

bool Task::ITask::destroyed()
{
    return m_destroyed;
}

Task::ID Task::ITask::generateId()
{
    static std::atomic<Task::ID> s_count(1);
    return s_count.fetch_add(1);
}
