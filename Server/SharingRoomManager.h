#ifndef SHARINGROOMMANAGER_H
#define SHARINGROOMMANAGER_H

#include "UserConnection.h"
#include "Global/Component/ReadWriteLock/ReadWriteLock.h"
#include <map>

template <typename SharingRoom>
class SharingRoomManager
{
public:
    typedef std::shared_ptr<SharingRoom> SharingRoomPtr;

public:
    SharingRoomManager()
    {

    }

    SharingRoomPtr createRoom(User::ID userId, UserConnectionPtr owner)
    {
        SharingRoomPtr ret;

        m_lock.writeLock();
        auto p = m_rooms.insert({userId, {owner, ret}});
        if (p.second)
        {
            ret.reset(new SharingRoom(owner));
            p.first->second.second = ret;
        }
        m_lock.writeUnlock();

        return ret;
    }

    SharingRoomPtr getRoom(User::ID userId)
    {
        SharingRoomPtr ret;

        m_lock.readLock();
        auto it = m_rooms.find(userId);
        if (it != m_rooms.end())
        {
            ret = it->second.second;
        }
        m_lock.readUnlock();

        return ret;
    }

    SharingRoomPtr removeRoom(User::ID userId)
    {
        SharingRoomPtr ret;

        m_lock.writeLock();
        auto it = m_rooms.find(userId);
        if (it != m_rooms.end())
        {
            ret = it->second.second;
            m_rooms.erase(it);
        }
        m_lock.writeUnlock();

        return ret;
    }

private:
    std::map<User::ID, std::pair<UserConnectionPtr, SharingRoomPtr>> m_rooms;
    ReadWriteLock m_lock;
};

#endif // SHARINGROOMMANAGER_H
