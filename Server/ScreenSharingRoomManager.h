#ifndef SCREENSHARINGROOMMANAGER_H
#define SCREENSHARINGROOMMANAGER_H

#include "SharingRoomManager.h"
#include "SharingRoom.h"

class ScreenSharingRoomManager : public SharingRoomManager<SharingRoom>
{
public:
    static ScreenSharingRoomManager* share();

private:
    ScreenSharingRoomManager();

private:
    static ScreenSharingRoomManager s_this;
};

#endif // SCREENSHARINGROOMMANAGER_H
