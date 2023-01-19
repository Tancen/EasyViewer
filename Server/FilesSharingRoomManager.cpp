#include "FilesSharingRoomManager.h"

FilesSharingRoomManager FilesSharingRoomManager::s_this;

FilesSharingRoomManager *FilesSharingRoomManager::share()
{
    return &s_this;
}

FilesSharingRoomManager::FilesSharingRoomManager()
{

}
