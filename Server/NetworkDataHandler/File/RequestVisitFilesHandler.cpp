#include "RequestVisitFilesHandler.h"
#include "FilesSharingRoomManager.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/File/File.pb.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Error.h"
#include "Task/File/VisitFilesTask.h"
#include "Task/TaskManager.h"
#include "UsualParseProtobufDataMacro.h"

void NetworkDataHandler::RequestVisitFiles::handle(IConnectionPtr con, const unsigned char *data, size_t len)
{
    UserConnection* usrcon = dynamic_cast<UserConnection*>(con.get());
    if (!usrcon)
    {
        Logger::warning("%s:%d - !usrcon, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    if (usrcon->getRole() != GLOBAL_CONNECTION_ROLE_FILE_TRANSMISSION_VISITOR)
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

    if (!BlackList::share()->checkAccount(usrInfo->id))
    {
        con->disconnect();
        return;
    }

    USUAL_PARSE_PROTOBUF_DATA_MACRO(Global::Protocol::File::RequestVisitFiles, request, data, len, usrcon->getSecondCryptology())

    if (usrcon->getTargetUserId())
    {
        Logger::warning("%s:%d - usrcon->getTargetUserId(), remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    if (!request.user_id())
    {
        Logger::warning("%s:%d - !request.user_id(), remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    if (request.user_id() == usrInfo->id)
    {
        Global::Protocol::File::ResponseVisitFiles response;
        response.set_err_code(GLOBAL_PROTOCOL_ERR_INVALID_PARAMS);
        response.set_err_string("can't connect youself");
        EasyIO::ByteBuffer data;
        GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISIT_FILES, response, data, usrcon->getSecondCryptology())
        usrcon->send(data);
        return;
    }

    usrcon->setTargetUserId(request.user_id());

    Task::Manager::share()->dispatch(Task::ITaskPtr(new Task::VisitFiles(
                        usrcon->shared_from_this(), request.user_id(),
                                request.auth_string())), 5000);
}
