#include "PublishScreenFrameHandler.h"
#include "ScreenSharingRoomManager.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/Screen/Screen.pb.h"
#include "Global/Protocol/Protocol.h"

void NetworkDataHandler::PublishScreenFrame::handle(IConnectionPtr con, const unsigned char *data, size_t len)
{
    UserConnection* usrcon = dynamic_cast<UserConnection*>(con.get());
    if (!usrcon)
    {
        Logger::warning("%s:%d - !usrcon, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    if ((usrcon->getRole() & GLOBAL_CONNECTION_ROLE_SCREEN_GUARD) != GLOBAL_CONNECTION_ROLE_SCREEN_GUARD)
    {
        Logger::warning("%s:%d - check role failed, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    const Account *usrInfo = usrcon->getBindingUser();
    if (!usrInfo)
    {
        Logger::warning("%s:%d - !usrInfo, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    auto room = ScreenSharingRoomManager::share()->getRoom(usrInfo->id);
    assert(room.get());
    auto viewers = room->getViewers();
    for (auto& v : viewers)
    {
        EasyIO::ByteBuffer buf;
        GLOBAL_PROTO_WRAPPER_DESC2(GLOBAL_PROTOCOL_NETWORK_TAG_PUBLISH_SCREEN_FRAME, data, len, buf)
        v->send(buf);
    }
}
