#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <mutex>
#include <QDateTime>
#include <map>
#include "ITask.h"
#include <vector>
#include <thread>
#include <condition_variable>

namespace Task
{

    class Manager
    {
        struct Item
        {
            ITaskPtr task;
            QDateTime executeAtTime;
            QDateTime maxStayAtTime;
            bool executed;
        };
        typedef std::shared_ptr<Item> ItemPtr;

        static bool compare(const ItemPtr& it1, const ItemPtr& it2)
        {
            const QDateTime *t1, *t2;

            if (it1->executed)
                t1 = &it1->maxStayAtTime;
            else
                t1 = &it1->executeAtTime;

            if (it2->executed)
                t2 = &it2->maxStayAtTime;
            else
                t2 = &it2->executeAtTime;

            return *t1 > *t2;
        }

    public:
        static Manager* share();
        static bool init(unsigned numWorker = 2);
        static void release();
        ~Manager();

        void exit();
        void waitForExited();

        bool dispatch(ITaskPtr task, unsigned int msTimeout);
        bool dispatch(ITaskPtr task, const QDateTime& executeAtTime);
        bool dispatch(ITaskPtr task, const QDateTime& executeAtTime,
                unsigned int msTimeout);
        bool dispatch(ITaskPtr task, const QDateTime& executeAtTime,
                QDateTime maxStayAtTime);

        ITaskPtr destroy(Task::ID taskID, Task::Certificate certificate);

    private:
        Manager(unsigned numWorker);
        void popHeap(std::vector<ItemPtr> &heap);
        void pushHeap(std::vector<ItemPtr> &heap, ItemPtr& it);

        void exec();

    private:
        std::map<ID, ItemPtr> m_mapItems;
        std::vector<ItemPtr> m_heapItems;
        std::vector<std::thread> m_workers;
        std::mutex m_mutex;
        bool m_exit = false;
        std::condition_variable m_cv;

        static Manager* s_this;
    };
}

#endif // TASK_MANAGER_H
