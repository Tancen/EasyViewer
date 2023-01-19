#ifndef SHARINGROOM_H
#define SHARINGROOM_H

#include "UserConnection.h"
#include <functional>
#include <set>

class SharingRoom
{
public:
    SharingRoom(UserConnectionPtr owner);

    UserConnectionPtr getOwner();
    bool addViewer(UserConnectionPtr userConnection, bool uniqueUserId = true);
    void removeViewer(UserConnectionPtr userConnection);

    std::set<UserConnectionPtr> getViewers();
    std::set<UserConnectionPtr> getViewers(User::ID userId);

    void setRoomEmptyCallback(std::function<void(SharingRoom* room)> enterEmptyCallback);
    void setRoomNonemptyCallback(std::function<void(SharingRoom* room)> leaveEmptyCallback);

private:
    std::recursive_mutex m_mutex;
    UserConnectionPtr m_owner;
    std::set<UserConnectionPtr> m_viewers;

    std::function<void(SharingRoom* room)> m_roomEmptyCallback;
    std::function<void(SharingRoom* room)> m_roomNonemptyCallback;
};

typedef std::shared_ptr<SharingRoom> SharingRoomPtr;

#endif // SHARINGROOM_H
