#include "EasyIOEventLoopGroup.h"

EasyIO::EventLoopGroup::EventLoopGroup(const std::vector<EasyIO::IEventLoopPtr> &workers)
    :   m_workers(workers),
        m_index(0)
{

}

EasyIO::EventLoopGroup::~EventLoopGroup()
{

}

const std::vector<EasyIO::IEventLoopPtr> &EasyIO::EventLoopGroup::getEventLoopGroup()
{
    return m_workers;
}

EasyIO::IEventLoop *EasyIO::EventLoopGroup::getNext()
{
    return m_workers.at(++m_index % m_workers.size()).get();
}
