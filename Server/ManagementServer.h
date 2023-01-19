#ifndef MANAGEMENTSERVER_H
#define MANAGEMENTSERVER_H

#include "ServerBase.h"

class ManagementServer : public ServerBase
{
    class CleanUserConnection : public NetworkDataHandler::INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };

public:
    static ManagementServer* share();

private:
    ManagementServer();
    ~ManagementServer();

    virtual void whenConnected(UserConnectionPtr con) override;
    virtual void whenDisconnected(UserConnectionPtr con) override;
    UserConnection::ReceivedDataHandlerFactory getReceivedDataHandlerFactory() override;

private:
    static ManagementServer s_this;
};

#endif // MANAGEMENTSERVER_H
