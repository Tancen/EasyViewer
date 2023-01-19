#ifndef EASYIOEVENTLOOPGROUP_H
#define EASYIOEVENTLOOPGROUP_H

#include <vector>
#include <atomic>
#include "EasyIOIEventLoop.h"

namespace EasyIO
{
    class EventLoopGroup
    {
    public:
        EventLoopGroup(const std::vector<IEventLoopPtr>& workers);
        ~EventLoopGroup();

    const std::vector<IEventLoopPtr>& getEventLoopGroup();

    IEventLoop* getNext();

    private:
        std::vector<IEventLoopPtr> m_workers;
        std::atomic<unsigned> m_index;
    };
    typedef std::shared_ptr<EventLoopGroup> EventLoopGroupPtr;
}

#endif
