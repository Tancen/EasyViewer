#ifndef FILESSHARINGROOMMANAGER_H
#define FILESSHARINGROOMMANAGER_H

#include "SharingRoomManager.h"
#include "SharingRoom.h"

class FilesSharingRoomManager : public SharingRoomManager<SharingRoom>
{
public:
    static FilesSharingRoomManager* share();

private:
    FilesSharingRoomManager();

private:
    static FilesSharingRoomManager s_this;
};

#endif // FILESSHARINGROOMMANAGER_H
