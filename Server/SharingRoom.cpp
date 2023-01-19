#include "SharingRoom.h"
#include <assert.h>

SharingRoom::SharingRoom(UserConnectionPtr owner)
    :   m_owner(owner)
{
    assert(owner.get());
}

UserConnectionPtr SharingRoom::getOwner()
{
    return m_owner;
}

bool SharingRoom::addViewer(UserConnectionPtr userConnection, bool uniqueUserId)
{
    bool b = false;
    m_mutex.lock();

    bool empty = m_viewers.empty();
    do
    {
        if(uniqueUserId && !getViewers(userConnection->getBindingUser()->id).empty())
            break;

        b = m_viewers.insert(userConnection).second;
    } while (0);
    if (empty && !m_viewers.empty() && m_roomNonemptyCallback)
        m_roomNonemptyCallback(this);

    m_mutex.unlock();

    return b;
}

void SharingRoom::removeViewer(UserConnectionPtr userConnection)
{
    m_mutex.lock();
    m_viewers.erase(userConnection);
    if (m_viewers.empty() && m_roomEmptyCallback)
        m_roomEmptyCallback(this);
    m_mutex.unlock();
}

std::set<UserConnectionPtr> SharingRoom::getViewers()
{
    std::set<UserConnectionPtr> ret;

    m_mutex.lock();
    ret = m_viewers;
    m_mutex.unlock();

    return ret;
}

std::set<UserConnectionPtr> SharingRoom::getViewers(User::ID userId)
{
    std::set<UserConnectionPtr> ret;

    m_mutex.lock();
    for (auto it : m_viewers)
    {
        if (it->getBindingUser()->id == userId)
            ret.insert(it);
    }
    m_mutex.unlock();

    return ret;
}

void SharingRoom::setRoomEmptyCallback(std::function<void (SharingRoom *)> enterEmptyCallback)
{
    m_mutex.lock();
    m_roomEmptyCallback = enterEmptyCallback;
    m_mutex.unlock();
}

void SharingRoom::setRoomNonemptyCallback(std::function<void (SharingRoom *)> leaveEmptyCallback)
{
    m_mutex.lock();
    m_roomNonemptyCallback = leaveEmptyCallback;
    m_mutex.unlock();
}
