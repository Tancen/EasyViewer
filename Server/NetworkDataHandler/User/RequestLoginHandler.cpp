#include "Global/Protocol/User/User.pb.h"
#include "Global/Protocol/Screen/Control.pb.h"
#include "Global/Component/Logger/Logger.h"
#include "UserConnection.h"
#include "RequestLoginHandler.h"
#include "AccountManager.h"
#include "Global/Protocol/Error.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Define.h"
#include "ScreenSharingRoomManager.h"
#include "FilesSharingRoomManager.h"
#include "TerminalSharingRoomManager.h"
#include "UsualParseProtobufDataMacro.h"
#include <inttypes.h>

void NetworkDataHandler::RequestLogin::handle(IConnectionPtr con, const unsigned char *data, size_t len)
{
    UserConnection* usrcon = dynamic_cast<UserConnection*>(con.get());
    if (!usrcon)
    {
        Logger::warning("%s:%d - !usrcon, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    int err = GLOBAL_PROTOCOL_ERR_NO_ERROR;
    std::string errorString;
    Account info;
    Global::Protocol::User::ResponseLogin response;
    do
    {
        if (!BlackList::share()->checkIPAddress(con->peerIP()))
        {
            err = GLOBAL_PROTOCOL_ERR_IP_ADDRESS_BLOCKED;
            errorString = Global::Protocol::formatError(err);
            break;
        }

        if (usrcon->getBindingUser() != nullptr)
        {
            Logger::warning("%s:%d - usrcon->getBindingUser() != nullptr, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                            con->peerIP().c_str(), con->peerPort());
            err = GLOBAL_PROTOCOL_ERR_REPEAT_LOGIN;
            break;
        }

        USUAL_PARSE_PROTOBUF_DATA_MACRO(Global::Protocol::User::RequestLogin, request, data, len, usrcon->getFirstCryptology())
        if (request.secret_key().empty())
        {
            err = GLOBAL_PROTOCOL_ERR_INVALID_PARAMS;
            errorString = "invalid member secret key";
            BlackList::share()->increaseIPAddress(con->peerIP());
            break;
        }

        std::unique_ptr<Cryptology::AES<EasyIO::ByteBuffer>> aes = Cryptology::AES<EasyIO::ByteBuffer>::newInstance(request.secret_key(), &errorString);
        if (!aes)
        {
            err = GLOBAL_PROTOCOL_ERR_OPERATE_FAILED;
            break;
        }

        if (request.role() == GLOBAL_CONNECTION_ROLE_MANAGER)
        {
            if (!AccountManager::share()->checkAdminPassword(request.password()))
            {
                err = GLOBAL_PROTOCOL_ERR_PASSWORD_INCORRECT;
                errorString = Global::Protocol::formatError(err);
                BlackList::share()->increaseIPAddress(con->peerIP());
                break;
            }
            usrcon->L = 1024 * 1024 * 1024; // 1 GB/s
        }
        else
        {
            err = AccountManager::share()->getAccount(request.account(), info);
            if (err != GLOBAL_PROTOCOL_ERR_NO_ERROR)
            {
                errorString = Global::Protocol::formatError(err);
                break;
            }

            if (!BlackList::share()->checkAccount(info.id))
            {
                err = GLOBAL_PROTOCOL_ERR_ACCOUNT_BLOCKED;
                errorString = Global::Protocol::formatError(err);
                break;
            }

            if (info.disabled)
            {
                err = GLOBAL_PROTOCOL_ERR_ACCOUNT_DISABLED;
                errorString = Global::Protocol::formatError(err);
                break;
            }

            if (!AccountManager::share()->checkPassword(info, request.password()))
            {
                err = GLOBAL_PROTOCOL_ERR_PASSWORD_INCORRECT;
                errorString = Global::Protocol::formatError(err);
                BlackList::share()->increaseIPAddress(con->peerIP());
                break;
            }

            //check high
            unsigned int h = (request.role() >> 16) & 0xffff;
            unsigned int h1 = h;
            h1 |= h1 >> 1;
            h1 |= h1 >> 2;
            h1 |= h1 >> 4;
            h1 |= h1 >> 8;
            h1++;
            h1 >>= 1;
            if (h1 != h)
            {
                Logger::warning("%s:%d - not allow multi high role, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                                con->peerIP().c_str(), con->peerPort());
                err = GLOBAL_PROTOCOL_ERR_INVALID_PARAMS;
                errorString = "not allow multi high role";
                break;
            }

            // check low
            if (h == GLOBAL_CONNECTION_ROLE_FLAG_VISITOR || h == GLOBAL_CONNECTION_ROLE_FLAG_MANAGER)
            {
                unsigned int l = request.role() & 0xffff;
                unsigned int l1 = l;
                l1 |= l1 >> 1;
                l1 |= l1 >> 2;
                l1 |= l1 >> 4;
                l1 |= l1 >> 8;
                l1++;
                l1 >>= 1;
                if (l != l1)
                {
                    Logger::warning("%s:%d - not allow multi low role with non-guard high role, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                                    con->peerIP().c_str(), con->peerPort());
                    err = GLOBAL_PROTOCOL_ERR_INVALID_PARAMS;
                    errorString = "not allow multi low role with non-guard high role";
                    break;
                }
            }

            if (request.role() & GLOBAL_CONNECTION_ROLE_FLAG_GUARD)
            {
                do
                {
                    if (request.role() & GLOBAL_CONNECTION_ROLE_SCREEN)
                    {
                        auto room = ScreenSharingRoomManager::share()->createRoom(info.id, usrcon->shared_from_this());
                        if (!room)
                        {
                            Logger::warning("%s:%d - !createRoom, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                                            con->peerIP().c_str(), con->peerPort());
                            err = GLOBAL_PROTOCOL_ERR_UNKNOW;
                            errorString = Global::Protocol::formatError(err);
                            break;
                        }
                        else
                        {
                            room->setRoomEmptyCallback([](SharingRoom* room)
                            {
                                Global::Protocol::Screen::EnableAutoPublishDesktop request;
                                request.set_enable(false);

                                EasyIO::ByteBuffer data;
                                GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_SET_AUTO_PUBLISH_DESKTOP, request, data, room->getOwner()->getSecondCryptology())
                                room->getOwner()->send(data);
                            });

                            room->setRoomNonemptyCallback([](SharingRoom* room)
                            {
                                Global::Protocol::Screen::EnableAutoPublishDesktop request;
                                request.set_enable(true);

                                EasyIO::ByteBuffer data;
                                GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_SET_AUTO_PUBLISH_DESKTOP, request, data, room->getOwner()->getSecondCryptology())
                                room->getOwner()->send(data);
                            });
                        }
                    }
                    if (request.role() & GLOBAL_CONNECTION_ROLE_FILE_TRANSMISSION)
                    {
                        if (!FilesSharingRoomManager::share()->createRoom(info.id, usrcon->shared_from_this()))
                        {
                            Logger::warning("%s:%d - !createRoom, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                                            con->peerIP().c_str(), con->peerPort());
                            err = GLOBAL_PROTOCOL_ERR_UNKNOW;
                            errorString = Global::Protocol::formatError(err);
                            break;
                        }
                    }
                    if (request.role() &  GLOBAL_CONNECTION_ROLE_TERMINAL)
                    {
                        if (!TerminalSharingRoomManager::share()->createRoom(info.id, usrcon->shared_from_this()))
                        {
                            Logger::warning("%s:%d - !createRoom, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                                            con->peerIP().c_str(), con->peerPort());
                            err = GLOBAL_PROTOCOL_ERR_UNKNOW;
                            errorString = Global::Protocol::formatError(err);
                            break;
                        }
                    }


                } while (0);

                if (err != GLOBAL_PROTOCOL_ERR_NO_ERROR)
                {
                    {
                        auto room = ScreenSharingRoomManager::share()->removeRoom(info.id);
                        if (room.get())
                        {
                            auto viewers = room->getViewers();
                            for (auto & viewer : viewers)
                                viewer->disconnect();
                        }
                    }
                    {
                        auto room = FilesSharingRoomManager::share()->removeRoom(info.id);
                        if (room.get())
                        {
                            auto viewers = room->getViewers();
                            for (auto & viewer : viewers)
                                viewer->disconnect();
                        }
                    }
                    {
                        auto room = TerminalSharingRoomManager::share()->removeRoom(info.id);
                        if (room.get())
                        {
                            auto viewers = room->getViewers();
                            for (auto & viewer : viewers)
                                viewer.con->disconnect();
                        }
                    }
                }
            }
        }

        if (err == GLOBAL_PROTOCOL_ERR_NO_ERROR)
        {
            usrcon->bindUser(info, request.role());
            usrcon->setSecondCryptology(std::move(aes));
        }
    }
    while (0);


    EasyIO::ByteBuffer responseData;
    response.set_err_code(err);
    response.set_err_string(errorString);
    response.set_user_id(info.id);

    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LOGIN, response, responseData, usrcon->getFirstCryptology())
    usrcon->send(responseData);

}
