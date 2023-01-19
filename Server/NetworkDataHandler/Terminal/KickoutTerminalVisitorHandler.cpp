#include "KickoutTerminalVisitorHandler.h"
#include "TerminalSharingRoomManager.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/Terminal/Terminal.pb.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Error.h"
#include "UsualParseProtobufDataMacro.h"

void NetworkDataHandler::KickoutTerminalVisitor::handle(IConnectionPtr con, const unsigned char *data, size_t len)
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

    USUAL_PARSE_PROTOBUF_DATA_MACRO(Global::Protocol::Terminal::KickoutVisitor, request, data, len, usrcon->getSecondCryptology())

    do
    {
        auto room = TerminalSharingRoomManager::share()->getRoom(usrcon->getBindingUser()->id);
        if (!room.get())
            break;

        auto viewers = room->getViewers();
        for (auto it : viewers)
        {
            if (it.terminalId == request.terminal_id())
            {
                it.con->disconnect();
                break;
            }
        }
        return;
    } while (0);

    con->disconnect();
}
