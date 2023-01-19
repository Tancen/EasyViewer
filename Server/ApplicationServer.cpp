#include "ApplicationServer.h"
#include "ScreenSharingRoomManager.h"
#include "FilesSharingRoomManager.h"
#include "TerminalSharingRoomManager.h"
#include <cinttypes>
#include "Global/Define.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Component/Logger/Logger.h"
#include "NetworkDataHandler/Factory.h"
#include "RequestProcessor.h"
#include "Global/Protocol/Terminal/Terminal.pb.h"

#define TAG_CLEAN   0

ApplicationServer ApplicationServer::s_this;

ApplicationServer *ApplicationServer::share()
{
    return &s_this;
}

ApplicationServer::ApplicationServer()
    : ServerBase(std::thread::hardware_concurrency())
{

}

ApplicationServer::~ApplicationServer()
{

}

void ApplicationServer::whenConnected(UserConnectionPtr con)
{

}

void ApplicationServer::whenDisconnected(UserConnectionPtr con)
{
    RequestProcessor::share()->push(con, TAG_CLEAN, EasyIO::ByteBuffer(),
                NetworkDataHandler::INetworkDataHandlerPtr(new CleanUserConnection(con)));
}

UserConnection::ReceivedDataHandlerFactory ApplicationServer::getReceivedDataHandlerFactory()
{
    return [](uint32_t tag)
    {
        return NetworkDataHandler::Factory::create(tag);
    };
}

ApplicationServer::CleanUserConnection::CleanUserConnection(UserConnectionPtr con)
    :   m_con(con)
{

}

void ApplicationServer::CleanUserConnection::handle(IConnectionPtr con, const unsigned char *data, size_t len)
{
    UserConnection* usrcon = dynamic_cast<UserConnection*>(con.get());
    assert(usrcon);
    const Account *info = usrcon->getBindingUser();
    if (info)
    {
        int role = usrcon->getRole();
        if (role & GLOBAL_CONNECTION_ROLE_FLAG_GUARD)
        {
            if (role & GLOBAL_CONNECTION_ROLE_SCREEN)
            {
                auto room = ScreenSharingRoomManager::share()->removeRoom(info->id);
                assert(room.get());

                auto viewers = room->getViewers();
                for (auto & viewer : viewers)
                    viewer->disconnect();
            }

            if (role & GLOBAL_CONNECTION_ROLE_FILE_TRANSMISSION)
            {
                auto room = FilesSharingRoomManager::share()->removeRoom(info->id);
                assert(room.get());

                auto viewers = room->getViewers();
                for (auto & viewer : viewers)
                    viewer->disconnect();
            }

            if (role & GLOBAL_CONNECTION_ROLE_TERMINAL)
            {
                auto room = TerminalSharingRoomManager::share()->removeRoom(info->id);
                assert(room);
                auto viewers = room->getViewers();
                for (auto & v : viewers)
                    v.con->disconnect();
            }
        }
        else if (role & GLOBAL_CONNECTION_ROLE_FLAG_VISITOR)
        {
            if (role & GLOBAL_CONNECTION_ROLE_SCREEN)
            {
                auto room = ScreenSharingRoomManager::share()->getRoom(usrcon->getTargetUserId());
                if (room.get())
                    room->removeViewer(usrcon->shared_from_this());
            }
            else if (role & GLOBAL_CONNECTION_ROLE_FILE_TRANSMISSION)
            {
                auto room = FilesSharingRoomManager::share()->getRoom(usrcon->getTargetUserId());
                if (room.get())
                    room->removeViewer(usrcon->shared_from_this());
            }
            else if (role & GLOBAL_CONNECTION_ROLE_TERMINAL)
            {
                auto room = TerminalSharingRoomManager::share()->getRoom(usrcon->getTargetUserId());
                if (room.get())
                {
                    std::string terminalId = room->removeViewer(usrcon->shared_from_this());
                    if (!terminalId.empty())
                    {
                        Global::Protocol::Terminal::CloseTerminal request;
                        request.set_terminal_id(terminalId);

                        EasyIO::ByteBuffer data;
                        GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CLOSE_TERMINAL, request, data, room->getOwner()->getSecondCryptology())
                        room->getOwner()->send(data);
                    }
                }
            }
        }

        Logger::info("%s:%d - %s:%d disconnected, userId %" PRId64 ", role %d",  __PRETTY_FUNCTION__, __LINE__,
                 con->peerIP().c_str(), con->peerPort(), info->id, usrcon->getRole());
    }

    usrcon->disconnect();
}
