#include "WriteCommandHandler.h"
#include "TerminalSharingRoomManager.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/Terminal/Terminal.pb.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Error.h"
#include "UsualParseProtobufDataMacro.h"

void NetworkDataHandler::WriteCommand::handle(IConnectionPtr con, const unsigned char *data, size_t len)
{
    UserConnection* usrcon = dynamic_cast<UserConnection*>(con.get());
    if (!usrcon)
    {
        Logger::warning("%s:%d - !usrcon, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    if (usrcon->getRole() != GLOBAL_CONNECTION_ROLE_TERMINAL_VISITOR)
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

    USUAL_PARSE_PROTOBUF_DATA_MACRO(Global::Protocol::Terminal::WriteCommand, request, data, len, usrcon->getSecondCryptology())

    do
    {
        auto room = TerminalSharingRoomManager::share()->getRoom(usrcon->getTargetUserId());
        if (!room.get())
            break;

        std::string terminalId = room->getTerminalId(usrcon->shared_from_this());
        if (terminalId.empty())
            break;

        request.set_user_id(usrInfo->id);
        request.set_terminal_id(terminalId);

        EasyIO::ByteBuffer data;
        GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_WRITE_COMMAND_TO_TERMINAL, request, data, room->getOwner()->getSecondCryptology())
        room->getOwner()->send(data);

        return;
    } while (0);

    con->disconnect();
}
