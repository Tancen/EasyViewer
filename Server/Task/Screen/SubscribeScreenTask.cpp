#include "SubscribeScreenTask.h"
#include "ScreenSharingRoomManager.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/Screen/Screen.pb.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Error.h"
#include "BlackList.h"
#include <inttypes.h>

Task::SubscribeScreen::SubscribeScreen(UserConnectionPtr src, User::ID partnerUserId, const std::string &authString, Task::Certificate certificate)
    :   ITask(certificate), m_src(src), m_partnerUserId(partnerUserId), m_authString(authString)
{

}

void Task::SubscribeScreen::execute()
{
    int errCode;

    do
    {
        auto room = ScreenSharingRoomManager::share()->getRoom(m_partnerUserId);
        if (!room)
        {
            errCode = GLOBAL_PROTOCOL_ERR_TARGET_UNREACHABLE;
            break;
        }

        Global::Protocol::Screen::RequestSubscribeScreen2 request;
        request.set_user_id(m_src->getBindingUser()->id);
        request.set_async_task_id(id());
        request.set_async_task_certificate(certificate());
        request.set_auth_string(m_authString);

        EasyIO::ByteBuffer data;
        GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_SUBSCRIBE_SCREEN2, request, data, room->getOwner()->getSecondCryptology())
        room->getOwner()->send(data);

        return;
    } while (0);

    feedback(errCode, Global::Protocol::formatError(errCode), "");
    this->destroy();
}

void Task::SubscribeScreen::timeout()
{
    int err = GLOBAL_PROTOCOL_ERR_TIMEOUT;
    feedback(err, Global::Protocol::formatError(err), "");
}

void Task::SubscribeScreen::handleResponse(UserConnectionPtr con, int errCode, const std::string &errString, const std::string& secret_key)
{
    std::string errString1 = errString;
    do
    {
        if (errCode == GLOBAL_PROTOCOL_ERR_NO_ERROR)
        {
            auto room = ScreenSharingRoomManager::share()->getRoom(m_partnerUserId);
            assert(room.get());

            if (!room->addViewer(m_src))
            {
                errCode = GLOBAL_PROTOCOL_ERR_REPEAT_LOGIN;
                errString1 = Global::Protocol::formatError(errCode);
                break;
            }
        }
        else if (errCode == GLOBAL_PROTOCOL_ERR_PASSWORD_INCORRECT)
        {
            Logger::info("%s:%d - %" PRId64 "[%s:%d] pass an incorrect password to visit the partner %" PRId64,
                         __PRETTY_FUNCTION__, __LINE__,
                         m_src->getBindingUser()->id, m_src->peerIP().c_str(), m_src->peerPort(),
                         m_partnerUserId);
            BlackList::share()->increaseAccount(*(m_src->getBindingUser()));
        }

    } while (0);
    feedback(errCode, errString1, secret_key);
}

void Task::SubscribeScreen::feedback(int errCode, const std::string &errString, const std::string& secret_key)
{
    Global::Protocol::Screen::ResponseSubscribeScreen response;
    response.set_err_code(errCode);
    response.set_err_string(errString);
    response.set_secret_key(secret_key);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_SUBSCRIBE_SCREEN, response, data, m_src->getSecondCryptology())
    m_src->send(data);
}
