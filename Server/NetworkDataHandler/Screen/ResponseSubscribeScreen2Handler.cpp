#include "ResponseSubscribeScreen2Handler.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/Screen/Screen.pb.h"
#include "Global/Protocol/Protocol.h"
#include "Task/TaskManager.h"
#include "Task/Screen/SubscribeScreenTask.h"
#include "UsualParseProtobufDataMacro.h"

void NetworkDataHandler::ResponseSubscribeScreen2::handle(IConnectionPtr con, const unsigned char *data, size_t len)
{
    UserConnection* usrcon = dynamic_cast<UserConnection*>(con.get());
    if (!usrcon)
    {
        Logger::warning("%s:%d - !usrcon, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    if ((usrcon->getRole() & GLOBAL_CONNECTION_ROLE_SCREEN_GUARD) != GLOBAL_CONNECTION_ROLE_SCREEN_GUARD)
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

    USUAL_PARSE_PROTOBUF_DATA_MACRO(Global::Protocol::Screen::ResponseSubscribeScreen2, response, data, len, usrcon->getSecondCryptology())


    auto task = Task::Manager::share()->destroy(response.async_task_id(), response.async_task_certificate());
    if (task.get())
    {
        Task::SubscribeScreen* rt = dynamic_cast<Task::SubscribeScreen*>(task.get());
        if (!rt)
        {
            Logger::warning("%s:%d - !rt, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                            con->peerIP().c_str(), con->peerPort());
            return;
        }
        rt->handleResponse(usrcon->shared_from_this(), response.err_code(), response.err_string(), response.secret_key());
    }
}
