#ifndef TASK_ITASK_H
#define TASK_ITASK_H

#include <mutex>
#include <QDateTime>

namespace Task
{
    typedef uint64_t ID;
    typedef uint32_t Certificate;

    class ITask
    {
    public:
        ITask(Task::Certificate certificate = rand());

        Task::ID id();
        Task::Certificate certificate();

        virtual void execute() = 0;
        virtual void timeout() = 0;

        void lock();
        void unlock();

        void destroy();
        bool destroyed();

    private:
        static ID generateId();

    protected:
        ID m_id;
        Certificate m_certificate;
        bool m_destroyed = false;
        std::recursive_mutex m_mutex;
    };
    typedef std::shared_ptr<ITask> ITaskPtr;

}

#endif // ITASK_H
