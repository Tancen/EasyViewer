#ifndef TERMINALSHARINGROOM_H
#define TERMINALSHARINGROOM_H

#include "UserConnection.h"
#include <set>

class TerminalSharingRoom
{
public:
    struct Viewer
    {
        UserConnectionPtr con;
        std::string terminalId;

        bool operator < (const Viewer& v) const
        {
            return con < v.con;
        }
    };

public:

    TerminalSharingRoom(UserConnectionPtr owner);
    ~TerminalSharingRoom();

    UserConnectionPtr getOwner();
    bool addViewer(UserConnectionPtr con, const std::string& terminalId);
    std::string removeViewer(UserConnectionPtr con);

    std::set<Viewer> getViewers();
    std::string getTerminalId(UserConnectionPtr con);

private:
    std::recursive_mutex m_mutex;
    UserConnectionPtr m_owner;
    std::set<Viewer> m_viewers;
};

typedef std::shared_ptr<TerminalSharingRoom> TerminalSharingRoomPtr;

#endif // TERMINALSHARINGROOM_H
