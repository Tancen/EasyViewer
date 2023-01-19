#include "SetClipboardTextHandler.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Screen/Control.pb.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "ScreenSharingRoomManager.h"

void NetworkDataHandler::SetClipboardText::handle(IConnectionPtr con, const unsigned char *data, size_t len)
{
    UserConnection* usrcon = dynamic_cast<UserConnection*>(con.get());
    if (!usrcon)
    {
        Logger::warning("%s:%d - !usrcon, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    const Account *userSelf = usrcon->getBindingUser();
    if (!userSelf)
    {
        Logger::warning("%s:%d - !usrInfo, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    auto send = [](UserConnection* con, const unsigned char *data, size_t len)
    {
        EasyIO::ByteBuffer buf;
        GLOBAL_PROTO_WRAPPER_DESC2(GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_SET_CLIPBOARD_TEXT, data, len, buf)
        con->send(buf);
    };

    if (usrcon->getRole() & GLOBAL_CONNECTION_ROLE_FLAG_VISITOR)   //viewer
    {
        User::ID targetUserId = usrcon->getTargetUserId();
        if (!targetUserId)
        {
            Logger::warning("%s:%d - !usrcon->getTargetUserId(), remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                            con->peerIP().c_str(), con->peerPort());
            con->disconnect();
            return;
        }

        auto room = ScreenSharingRoomManager::share()->getRoom(targetUserId);
        if (!room.get())
        {
            return;
        }
        send(room->getOwner().get(), data, len);
    }
    else  if (usrcon->getRole() & GLOBAL_CONNECTION_ROLE_FLAG_GUARD)  //guard
    {
        auto room = ScreenSharingRoomManager::share()->getRoom(userSelf->id);
        if (!room.get())
        {
            return;
        }

        auto viewer = room->getViewers();
        for (auto v : viewer)
            send(v.get(), data, len);
    }
    else
    {
        Logger::warning("%s:%d - usrcon->role[] is neither guard nor visitor, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        usrcon->getRole(), con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }
}
