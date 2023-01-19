#include "CreateTerminalTask.h"
#include "TerminalSharingRoomManager.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Error.h"
#include "BlackList.h"
#include <inttypes.h>

Task::CreateTerminal::CreateTerminal(UserConnectionPtr src, User::ID partnerUserId, const std::string &authString,
                                     short width, short height, Task::Certificate certificate)
    :   ITask(certificate), m_src(src), m_partnerUserId(partnerUserId), m_authString(authString), m_width(width), m_height(height)
{

}

void Task::CreateTerminal::execute()
{
    int errCode;

    do
    {
        auto room = TerminalSharingRoomManager::share()->getRoom(m_partnerUserId);
        if (!room)
        {
            errCode = GLOBAL_PROTOCOL_ERR_TARGET_UNREACHABLE;
            break;
        }

        Global::Protocol::Terminal::RequestCreateTerminal2 request;
        request.set_user_id(m_src->getBindingUser()->id);
        request.set_async_task_id(id());
        request.set_async_task_certificate(certificate());
        request.set_auth_string(m_authString);
        request.set_width(m_width);
        request.set_height(m_height);

        EasyIO::ByteBuffer data;
        GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_CREATE_TERMINAL2, request, data, room->getOwner()->getSecondCryptology())
        room->getOwner()->send(data);

        return;
    } while (0);

    feedback(errCode, Global::Protocol::formatError(errCode), "", "");
    this->destroy();
}

void Task::CreateTerminal::timeout()
{
    int err = GLOBAL_PROTOCOL_ERR_TIMEOUT;
    feedback(err, Global::Protocol::formatError(err), "", "");
}

void Task::CreateTerminal::handleResponse(UserConnectionPtr con, const Global::Protocol::Terminal::ResponseCreateTerminal2& response)
{
    int errCode = response.err_code();
    std::string errString = response.err_string();
    std::string terminalId = response.terminal_id();

    do
    {
        if (errCode == GLOBAL_PROTOCOL_ERR_NO_ERROR)
        {
            auto room = TerminalSharingRoomManager::share()->getRoom(m_partnerUserId);
            assert(room.get());

            if (!room->addViewer(m_src, response.terminal_id()))
            {
                errCode = GLOBAL_PROTOCOL_ERR_REPEAT_LOGIN;
                errString = Global::Protocol::formatError(errCode);
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

    if (response.err_code() == GLOBAL_PROTOCOL_ERR_NO_ERROR
            && errCode != GLOBAL_PROTOCOL_ERR_NO_ERROR)
    {
        Global::Protocol::Terminal::CloseTerminal request;
        request.set_terminal_id(response.terminal_id());

        EasyIO::ByteBuffer data;
        GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CLOSE_TERMINAL, request, data, con->getSecondCryptology())
        con->send(data);
    }

    feedback(errCode, errString, terminalId, response.secret_key());
}

void Task::CreateTerminal::feedback(int errCode, const std::string &errString, const std::string& terminalId, const std::string& secretKey)
{
    Global::Protocol::Terminal::ResponseCreateTerminal response;
    response.set_err_code(errCode);
    response.set_err_string(errString);
    response.set_terminal_id(terminalId);
    response.set_secret_key(secretKey);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_CREATE_TERMINAL, response, data, m_src->getSecondCryptology())
    m_src->send(data);
}
