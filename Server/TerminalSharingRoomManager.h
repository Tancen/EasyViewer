#ifndef TERMINALSHARINGROOMMANAGER_H
#define TERMINALSHARINGROOMMANAGER_H

#include "TerminalSharingRoom.h"
#include "SharingRoomManager.h"

class TerminalSharingRoomManager : public SharingRoomManager<TerminalSharingRoom>
{
public:
    static TerminalSharingRoomManager* share();

private:
    TerminalSharingRoomManager();

private:
    static TerminalSharingRoomManager s_this;
};

#endif // TERMINALSHARINGROOMMANAGER_H
