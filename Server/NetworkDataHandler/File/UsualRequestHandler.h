#ifndef NETWORKDATAHANDLE_USUALREQUESTHANDLER_H
#define NETWORKDATAHANDLE_USUALREQUESTHANDLER_H

#include "../INetworkDataHandler.h"
#include "FilesSharingRoomManager.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Error.h"
#include "UsualParseProtobufDataMacro.h"

namespace  NetworkDataHandler
{
    template <class RequestType, class ResponseType>
    class FillSEQ
    {
    public:
        static void fill( RequestType q, ResponseType& r)
        {
            r.set_seq(q.seq());
        }
    };

    template <class RequestType, class ResponseType>
    class NoFillSEQ
    {
    public:
        static void fill( RequestType q, ResponseType& r)
        {

        }
    };

    template <class RequestType, class ResponseType>
    class FillTaskId
    {
    public:
        static void fill( RequestType q, ResponseType& r)
        {
            r.set_task_id(q.task_id());
        }
    };

    template <class RequestType, class ResponseType>
    class NoFillTaskId
    {
    public:
        static void fill( RequestType q, ResponseType& r)
        {

        }
    };

    template <class RequestType, class ResponseType,
        template<class, class> class SEQFiller = FillSEQ,
        template<class, class> class TaskIdFiller = NoFillTaskId
        >
    class UsualRequest : public INetworkDataHandler
    {
    public:
        UsualRequest(int tagRequest, int tagResponse, int checkRole = GLOBAL_CONNECTION_ROLE_FILE_TRANSMISSION_VISITOR)
            :   m_tagRequest(tagRequest), m_tagResponse(tagResponse),
                m_checkRole(checkRole)
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

            USUAL_PARSE_PROTOBUF_DATA_MACRO(RequestType, request, data, len, usrcon->getSecondCryptology())

            int err = GLOBAL_PROTOCOL_ERR_NO_ERROR;
            do
            {
                UserConnectionPtr conDst;
                if (!(usrcon->getRole() & GLOBAL_CONNECTION_ROLE_FLAG_GUARD))   //viewer
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
                        err = GLOBAL_PROTOCOL_ERR_TARGET_UNREACHABLE;
                    }
                    request.set_user_id(userSelf->id);
                    conDst = room->getOwner();
                }
                else     //guard
                {
                    auto room = FilesSharingRoomManager::share()->getRoom(userSelf->id);
                    if (!room.get())
                    {
                        err = GLOBAL_PROTOCOL_ERR_TARGET_UNREACHABLE;
                    }
                    auto viewers = room->getViewers(request.user_id());
                    if (viewers.empty())
                    {
                        err = GLOBAL_PROTOCOL_ERR_TARGET_UNREACHABLE;
                        break;
                    }
                    request.set_user_id(userSelf->id);
                    conDst = *viewers.begin();
                }
                EasyIO::ByteBuffer buf;
                GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(m_tagRequest, request, buf, conDst->getSecondCryptology())
                conDst->send(buf);
                return;
            } while (0);

            ResponseType response;
            SEQFiller<RequestType, ResponseType>::fill(request, response);
            TaskIdFiller<RequestType, ResponseType>::fill(request, response);
            response.set_err_code(err);
            response.set_err_string(Global::Protocol::formatError(err));
            EasyIO::ByteBuffer buf;
            GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(m_tagResponse, response, buf, usrcon->getSecondCryptology())
            usrcon->send(buf);
        }

    private:
        int m_tagRequest;
        int m_tagResponse;
        int m_checkRole;
    };
}

#endif // USUALREQUESTHANDLER_H
