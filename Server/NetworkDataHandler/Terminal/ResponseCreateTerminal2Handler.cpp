#include "ResponseCreateTerminal2Handler.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/Terminal/Terminal.pb.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Error.h"
#include "Task/TaskManager.h"
#include "Task/Terminal/CreateTerminalTask.h"
#include "UsualParseProtobufDataMacro.h"

void NetworkDataHandler::ResponseCreateTerminal2::handle(IConnectionPtr con, const unsigned char *data, size_t len)
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

    USUAL_PARSE_PROTOBUF_DATA_MACRO(Global::Protocol::Terminal::ResponseCreateTerminal2, response, data, len, usrcon->getSecondCryptology())

    auto task = Task::Manager::share()->destroy(response.async_task_id(), response.async_task_certificate());
    if (task.get())
    {
        Task::CreateTerminal* rt = dynamic_cast<Task::CreateTerminal*>(task.get());
        if (!rt)
        {
            Logger::warning("%s:%d - !rt, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                            con->peerIP().c_str(), con->peerPort());
            return;
        }
        rt->handleResponse(usrcon->shared_from_this(), response);
    }
    else
    {
        if (response.err_code() == GLOBAL_PROTOCOL_ERR_NO_ERROR)
        {
            Global::Protocol::Terminal::CloseTerminal request;
            request.set_terminal_id(response.terminal_id());

            EasyIO::ByteBuffer data;
            GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CLOSE_TERMINAL, request, data, usrcon->getSecondCryptology())
            usrcon->send(data);
        }
    }
}
