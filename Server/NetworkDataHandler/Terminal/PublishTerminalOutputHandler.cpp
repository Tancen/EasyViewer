#include "PublishTerminalOutputHandler.h"
#include "TerminalSharingRoomManager.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/Terminal/Terminal.pb.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Error.h"
#include "UsualParseProtobufDataMacro.h"

void NetworkDataHandler::PublishTerminalOutput::handle(IConnectionPtr con, const unsigned char *data, size_t len)
{
    UserConnection* usrcon = dynamic_cast<UserConnection*>(con.get());
    if (!usrcon)
    {
        Logger::warning("%s:%d - !usrcon, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    if ((usrcon->getRole() & GLOBAL_CONNECTION_ROLE_TERMINAL_GUARD) != GLOBAL_CONNECTION_ROLE_TERMINAL_GUARD)
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

    USUAL_PARSE_PROTOBUF_DATA_MACRO(Global::Protocol::Terminal::PublishTerminalOutput, publish, data, len, usrcon->getSecondCryptology())

    auto room = TerminalSharingRoomManager::share()->getRoom(usrInfo->id);
    assert(room.get());

    auto viewers = room->getViewers();
    for (auto& v : viewers)
    {
        auto info = v.con->getBindingUser();
        if (info && info->id == publish.user_id() && v.terminalId == publish.terminal_id())
        {
            EasyIO::ByteBuffer buf;
            GLOBAL_PROTO_WRAPPER_DESC2(GLOBAL_PROTOCOL_NETWORK_TAG_PUBLISH_TERMINAL_OUTPUT, data, len, buf)
            v.con->send(buf);
            return;
        }
    }
}
