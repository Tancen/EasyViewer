#ifndef TASK_CREATETERMINALTASK_H
#define TASK_CREATETERMINALTASK_H

#include "../ITask.h"
#include "UserConnection.h"
#include "Global/Define.h"
#include "Global/Protocol/Terminal/Terminal.pb.h"

namespace Task
{
    class CreateTerminal : public ITask
    {
    public:
        CreateTerminal(UserConnectionPtr src, User::ID partnerUserId, const std::string& authString,
                       short width, short height,
                       Task::Certificate certificate = rand());

        void execute();
        void timeout();
        void handleResponse(UserConnectionPtr con, const Global::Protocol::Terminal::ResponseCreateTerminal2& response);

    private:
        void feedback(int errCode, const std::string& errString, const std::string& terminalId, const std::string& secretKey);

    private:
        UserConnectionPtr m_src;
        User::ID m_partnerUserId;
        std::string m_authString;
        short m_width;
        short m_height;
    };
}

#endif // TASK_CREATETERMINALTASK_H
