#ifndef TASK_VISITFILESTASK_H
#define TASK_VISITFILESTASK_H

#include "../ITask.h"
#include "UserConnection.h"
#include "Global/Define.h"
#include "Global/Protocol/File/File.pb.h"

namespace Task
{
    class VisitFiles : public ITask
    {
    public:
        VisitFiles(UserConnectionPtr src, User::ID partnerUserId, const std::string& authString, Task::Certificate certificate = rand());

        void execute();
        void timeout();
        void handleResponse(UserConnectionPtr con, const Global::Protocol::File::ResponseVisitFiles2& response);

    private:
        void feedback(int errCode, const std::string& errString, const Global::Protocol::File::ResponseVisitFiles2* response);

    private:
        UserConnectionPtr m_src;
        User::ID m_partnerUserId;
        std::string m_authString;
    };
}

#endif // VISITFILESTASK_H
