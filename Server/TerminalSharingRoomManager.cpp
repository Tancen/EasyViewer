#include "TerminalSharingRoomManager.h"

TerminalSharingRoomManager TerminalSharingRoomManager::s_this;

TerminalSharingRoomManager *TerminalSharingRoomManager::share()
{
    return &s_this;
}

TerminalSharingRoomManager::TerminalSharingRoomManager()
{

}
