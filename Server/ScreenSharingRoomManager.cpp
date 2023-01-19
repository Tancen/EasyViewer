#include "ScreenSharingRoomManager.h"

ScreenSharingRoomManager ScreenSharingRoomManager::s_this;

ScreenSharingRoomManager *ScreenSharingRoomManager::share()
{
    return &s_this;
}

ScreenSharingRoomManager::ScreenSharingRoomManager()
{

}
