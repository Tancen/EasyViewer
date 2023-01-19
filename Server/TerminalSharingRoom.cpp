#include "TerminalSharingRoom.h"
#include <assert.h>

TerminalSharingRoom::TerminalSharingRoom(UserConnectionPtr owner)
    :   m_owner(owner)
{

}

TerminalSharingRoom::~TerminalSharingRoom()
{

}

UserConnectionPtr TerminalSharingRoom::getOwner()
{
    return m_owner;
}

bool TerminalSharingRoom::addViewer(UserConnectionPtr con, const std::string &terminalId)
{
    Viewer v;
    v.con = con;
    v.terminalId = terminalId;

    m_mutex.lock();
    bool b = m_viewers.insert(v).second;
    m_mutex.unlock();

    return b;
}

std::string TerminalSharingRoom::removeViewer(UserConnectionPtr con)
{
    std::string ret;
    Viewer v;
    v.con = con;

    m_mutex.lock();
    auto it = m_viewers.find(v);
    if (it != m_viewers.end())
    {
        ret = it->terminalId;
        m_viewers.erase(it);
    }
    m_mutex.unlock();

    return ret;
}

std::set<TerminalSharingRoom::Viewer> TerminalSharingRoom::getViewers()
{
    std::set<Viewer> ret;

    m_mutex.lock();
    ret = m_viewers;
    m_mutex.unlock();

    return ret;
}

std::string TerminalSharingRoom::getTerminalId(UserConnectionPtr con)
{
    std::string ret;
    Viewer v;
    v.con = con;

    m_mutex.lock();
    auto it = m_viewers.find(v);
    if (it != m_viewers.end())
        ret = it->terminalId;
    m_mutex.unlock();

    return ret;
}
