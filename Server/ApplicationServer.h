#ifndef APPLICATIONSERVER_H
#define APPLICATIONSERVER_H

#include "ServerBase.h"

class ApplicationServer : public ServerBase
{
    class CleanUserConnection : public NetworkDataHandler::INetworkDataHandler
    {
    public:
        CleanUserConnection(UserConnectionPtr con);
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);

    private:
        UserConnectionPtr m_con;
    };

public:
    static ApplicationServer* share();

private:
    ApplicationServer();
    ~ApplicationServer();

    virtual void whenConnected(UserConnectionPtr con) override;
    virtual void whenDisconnected(UserConnectionPtr con) override;
    UserConnection::ReceivedDataHandlerFactory getReceivedDataHandlerFactory() override;

private:
    static ApplicationServer s_this;
};

#endif // APPLICATIONSERVER_H
