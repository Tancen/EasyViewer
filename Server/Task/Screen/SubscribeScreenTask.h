#ifndef TASK_SUBSCRIBESCREENTASK_H
#define TASK_SUBSCRIBESCREENTASK_H

#include "../ITask.h"
#include "UserConnection.h"
#include "Global/Define.h"

namespace Task
{
    class SubscribeScreen : public ITask
    {
    public:
        SubscribeScreen(UserConnectionPtr src, User::ID partnerUserId, const std::string& authString, Task::Certificate certificate = rand());

        void execute();
        void timeout();
        void handleResponse(UserConnectionPtr con, int errCode, const std::string& errString, const std::string& secret_key);

    private:
        void feedback(int errCode, const std::string& errString, const std::string& secret_key);

    private:
        UserConnectionPtr m_src;
        User::ID m_partnerUserId;
        std::string m_authString;
    };
}

#endif // TASK_SUBSCRIBESCREENTASK_H
