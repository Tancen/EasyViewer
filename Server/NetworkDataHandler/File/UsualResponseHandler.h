#ifndef NETWORKDATAHANDLE_USUALRESPONSEHANDLER_H
#define NETWORKDATAHANDLE_USUALRESPONSEHANDLER_H

#include "../INetworkDataHandler.h"
#include "FilesSharingRoomManager.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/File/File.pb.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Error.h"
#include "UsualParseProtobufDataMacro.h"

namespace  NetworkDataHandler
{
    template <class ResponseType>
    class UsualResponseHandler : public INetworkDataHandler
    {
    public:
        UsualResponseHandler(int tagResponse, int checkRole = GLOBAL_CONNECTION_ROLE_FILE_TRANSMISSION_GUARD)
            : m_tagResponse(tagResponse), m_checkRole(checkRole)
        {

        }
        void handle(IConnectionPtr con, const unsigned char* data, size_t len)
        {
            UserConnection* usrcon = dynamic_cast<UserConnection*>(con.get());
            if (!usrcon)
            {
                Logger::warning("%s:%d - !usrcon, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                                con->peerIP().c_str(), con->peerPort());
                con->disconnect();
                return;
            }

            if (m_checkRole && ((usrcon->getRole() & m_checkRole) != m_checkRole))
            {
                Logger::warning("%s:%d - check role failed, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
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

            USUAL_PARSE_PROTOBUF_DATA_MACRO(ResponseType, response, data, len, usrcon->getSecondCryptology())

            do
            {
                UserConnectionPtr conDst;
                if (!(usrcon->getRole() & GLOBAL_CONNECTION_ROLE_FLAG_GUARD))
                {
                    User::ID targetUserId = usrcon->getTargetUserId();
                    if (!targetUserId)
                    {
                        Logger::warning("%s:%d - !usrcon->getTargetUserId(), remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                                        con->peerIP().c_str(), con->peerPort());
                        con->disconnect();
                        return;
                    }

                    auto room = FilesSharingRoomManager::share()->getRoom(targetUserId);
                    if (!room.get())
                    {
                        Logger::warning("%s:%d - !room.get(), remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                                        con->peerIP().c_str(), con->peerPort());
                        break;
                    }
                    conDst = room->getOwner();
                }
                else
                {
                    auto room = FilesSharingRoomManager::share()->getRoom(userSelf->id);
                    if (!room.get())
                    {
                        Logger::warning("%s:%d - !room.get(), remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                                        con->peerIP().c_str(), con->peerPort());
                        break;
                    }
                    auto viewers = room->getViewers(response.user_id());
                    if (viewers.empty())
                    {
                        Logger::warning("%s:%d - viewers.empty(), remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                                        con->peerIP().c_str(), con->peerPort());
                        break;
                    }
                    conDst = *viewers.begin();
                }
                EasyIO::ByteBuffer buf;
                GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(m_tagResponse, response, buf, conDst->getSecondCryptology())
                conDst->send(buf);
                return;
            } while (0);
        }

    private:
        int m_tagResponse;
        int m_checkRole;
    };
}


#endif // NETWORKDATAHANDLE_USUALRESPONSEHANDLER_H
