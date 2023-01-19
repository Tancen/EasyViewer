#include "VisitFilesTask.h"
#include "FilesSharingRoomManager.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/File/File.pb.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Error.h"
#include "BlackList.h"
#include <inttypes.h>

Task::VisitFiles::VisitFiles(UserConnectionPtr src, User::ID partnerUserId, const std::string &authString, Task::Certificate certificate)
    :   ITask(certificate), m_src(src), m_partnerUserId(partnerUserId), m_authString(authString)
{

}

void Task::VisitFiles::execute()
{
    int errCode;

    do
    {
        auto room = FilesSharingRoomManager::share()->getRoom(m_partnerUserId);
        if (!room)
        {
            errCode = GLOBAL_PROTOCOL_ERR_TARGET_UNREACHABLE;
            break;
        }

        Global::Protocol::File::RequestVisitFiles2 request;
        request.set_user_id(m_src->getBindingUser()->id);
        request.set_async_task_id(id());
        request.set_async_task_certificate(certificate());
        request.set_auth_string(m_authString);

        EasyIO::ByteBuffer data;
        GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISIT_FILES2, request, data, room->getOwner()->getSecondCryptology())
        room->getOwner()->send(data);

        return;
    } while (0);

    feedback(errCode, Global::Protocol::formatError(errCode), nullptr);
    this->destroy();
}

void Task::VisitFiles::timeout()
{
    int err = GLOBAL_PROTOCOL_ERR_TIMEOUT;
    feedback(err, Global::Protocol::formatError(err), nullptr);
}

void Task::VisitFiles::handleResponse(UserConnectionPtr con, const Global::Protocol::File::ResponseVisitFiles2& response)
{
    int errCode = response.err_code();
    std::string errString = response.err_string();
    do
    {
        if (errCode == GLOBAL_PROTOCOL_ERR_NO_ERROR)
        {
            auto room = FilesSharingRoomManager::share()->getRoom(m_partnerUserId);
            assert(room.get());

            if (!room->addViewer(m_src))
            {
                errCode = GLOBAL_PROTOCOL_ERR_REPEAT_LOGIN;
                errString = Global::Protocol::formatError(errCode);
                break;
            }
            ;
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
    feedback(errCode, errString, &response);
}

void Task::VisitFiles::feedback(int errCode, const std::string &errString, const Global::Protocol::File::ResponseVisitFiles2* response)
{
    Global::Protocol::File::ResponseVisitFiles response2;
    response2.set_err_code(errCode);
    response2.set_err_string(errString);
    if (errCode == GLOBAL_PROTOCOL_ERR_NO_ERROR && response)
    {
        for (int i = 0; i < response->default_dirs_size(); i++)
            response2.add_default_dirs(response->default_dirs(i));

        for (int i = 0; i < response->entries_size(); i++)
        {
            auto e2 = response2.add_entries();
            auto e = response->entries(i);
            e2->set_name(e.name());
            e2->set_size(e.size());
            e2->set_directory(e.directory());
            e2->set_is_directory(e.is_directory());
            e2->set_last_modifying_time(e.last_modifying_time());
        }
    }

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISIT_FILES, response2, data, m_src->getSecondCryptology())
    m_src->send(data);
}
