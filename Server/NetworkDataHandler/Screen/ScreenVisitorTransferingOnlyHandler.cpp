#include "ScreenVisitorTransferingOnlyHandler.h"
#include "ScreenSharingRoomManager.h"
#include "Global/Protocol/Protocol.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"

NetworkDataHandler::ScreenVisitorTransferingOnly::ScreenVisitorTransferingOnly(int tag)
    : m_tag(tag)
{

}

void NetworkDataHandler::ScreenVisitorTransferingOnly::handle(IConnectionPtr con, const unsigned char *data, size_t len)
{
    UserConnection* usrcon = dynamic_cast<UserConnection*>(con.get());
    if (!usrcon)
    {
        Logger::warning("%s:%d - !usrcon, tag[%d], remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        m_tag, con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    if (usrcon->getRole() != GLOBAL_CONNECTION_ROLE_SCREEN_VISITOR)
    {
        Logger::warning("%s:%d - role[%d] != GLOBAL_CONNECTION_ROLE_SCREEN_VISITOR,  tag[%d], remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        usrcon->getRole(), m_tag, con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    const Account *usrInfo = usrcon->getBindingUser();
    if (!usrInfo)
    {
        Logger::warning("%s:%d - !usrInfo,  tag[%d], remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        m_tag, con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    auto room = ScreenSharingRoomManager::share()->getRoom(usrcon->getTargetUserId());
    if(room.get())
    {
        auto owner = room->getOwner();
        assert(owner.get());

        EasyIO::ByteBuffer buf;
        GLOBAL_PROTO_WRAPPER_DESC2(m_tag, data, len, buf)
        owner->send(buf);

    }
}
