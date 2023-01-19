#include "TerminalGuardAbility.h"
#include "Global/Define.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Terminal/Terminal.pb.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/Error.h"
#include "AuthChecker.h"
#include "UsualParseProtobufDataMacro.h"

using namespace std::placeholders;

TerminalGuardAbility::TerminalGuardAbility()
        : m_terminalManager(nullptr)
{
        m_terminalManager = TerminalManager::create();
        if (!m_terminalManager)
        {
            Logger::error("%s:%d - init TerminalManager failed[%d: %s]",  __PRETTY_FUNCTION__, __LINE__);
            return;
        }

        m_terminalManager->onTerminalOutputted
                = std::bind(&TerminalGuardAbility::publishTerminalOutput, this, _1, _2, _3, _4);
        m_terminalManager->onTerminalClosed = std::bind(&TerminalGuardAbility::kickoutVisitor, this, _1, _2);
}

TerminalGuardAbility::~TerminalGuardAbility()
{
    delete m_terminalManager;
}

int TerminalGuardAbility::role()
{
    return GLOBAL_CONNECTION_ROLE_TERMINAL;
}

bool TerminalGuardAbility::handleCompleteData(unsigned tag, const unsigned char *data, size_t len)
{
    switch (tag)
    {
    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_CREATE_TERMINAL2:
        handleCommandOfCreateTerminal(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_WRITE_COMMAND_TO_TERMINAL:
        handleCommandOfWriteCommand(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESIZE_TERMINAL:
        handleCommandOfResizeTerminal(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CLOSE_TERMINAL:
        handleCommandOfCloseTerminal(data, len);
        return true;
    }

    return false;
}

void TerminalGuardAbility::handleCommandOfCreateTerminal(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Terminal::RequestCreateTerminal2, request, data, len, aes())

    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    Global::Protocol::Terminal::ResponseCreateTerminal2 response;
    int err = GLOBAL_PROTOCOL_ERR_NO_ERROR;
    if (!AuthChecker::share()->test(request.auth_string()))
    {
        err =  GLOBAL_PROTOCOL_ERR_PASSWORD_INCORRECT;
    }
    else
    {
        if (!m_terminalManager)
        {
             err = GLOBAL_PROTOCOL_ERR_UNSUPPORTED;
        }
        else
        {
            bool isOk;
            std::string id;
            id = m_terminalManager->createTerminal(request.user_id(), request.width(), request.height(), &isOk);
            if (!isOk)
            {
                err = GLOBAL_PROTOCOL_ERR_UNSUPPORTED;
            }
            else
            {
                response.set_terminal_id(id);
                response.set_secret_key(aes()->key());
            }
        }
    }
    response.set_async_task_id(request.async_task_id());
    response.set_async_task_certificate(request.async_task_certificate());
    response.set_err_code(err);
    response.set_err_string(Global::Protocol::formatError(err));

    EasyIO::ByteBuffer buf;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_CREATE_TERMINAL2, response, buf, aes())
    connection()->send(buf);
}

void TerminalGuardAbility::handleCommandOfWriteCommand(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Terminal::WriteCommand, request, data, len, aes())
    if (m_terminalManager)
        m_terminalManager->writeCommand(request.terminal_id(), request.command().c_str(), request.command().length());
}

void TerminalGuardAbility::handleCommandOfCloseTerminal(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Terminal::CloseTerminal, request, data, len, aes())

    if (m_terminalManager)
            m_terminalManager->closeTerminal(request.terminal_id());
}

void TerminalGuardAbility::handleCommandOfResizeTerminal(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Terminal::ResizeTerminal, request, data, len, aes())

    if (m_terminalManager)
            m_terminalManager->resizeTerminal(request.terminal_id(), request.width(), request.height());
}

void TerminalGuardAbility::publishTerminalOutput(User::ID userId, const std::string &terminalId, const char *src, size_t len)
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);

    Global::Protocol::Terminal::PublishTerminalOutput publish;
    publish.set_user_id(userId);
    publish.set_terminal_id(terminalId);
    publish.set_data(src, len);

    EasyIO::ByteBuffer buf;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_PUBLISH_TERMINAL_OUTPUT, publish, buf, aes())
    connection()->send(buf);
}

void TerminalGuardAbility::kickoutVisitor(User::ID userId, const std::string &terminalId)
{
    Global::Protocol::Terminal::KickoutVisitor request;
    request.set_terminal_id(terminalId);

    EasyIO::ByteBuffer buf;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_KICKOUT_VISITOR_TERMINAL, request, buf, aes())
            connection()->send(buf);
}

void TerminalGuardAbility::handleLoggedinEvent()
{

}

void TerminalGuardAbility::handleConnectionLostEvent()
{

}
