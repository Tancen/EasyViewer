#ifndef VIEWERBASE_H
#define VIEWERBASE_H

#include <string>
#include "Global/Component/EasyIO/EasyIO.h"
#include "Global/Component/Cryptology/Cryptology.hpp"
#include "Global/Define.h"

class ViewerBase
{
public:
    ViewerBase(const std::string& peerHost, const std::string& peerIP,
               unsigned short peerPort, const std::string& publicKey,
            const std::string& account, const std::string& password,
            User::ID partnerUserId, const std::string& authString);
    virtual ~ViewerBase();

    virtual std::pair<bool, std::string> init();

    virtual int role() = 0;

    void login();

    bool loggedIn();
    bool loggingIn();

    const std::string& peerHost();
    const std::string& peerIP();
    unsigned short peerPort();

    User::ID getSelfUserId();
    User::ID getPartnerUserId();
    const std::string& getAuthString();

protected:
    void send(EasyIO::ByteBuffer data);
    virtual bool handleCompleteData(unsigned tag, const unsigned char* data, size_t len ) = 0;

    virtual void whenConnecting(EasyIO::TCP::IConnection* con){}
    virtual void whenConnected(EasyIO::TCP::IConnection* con){}
    virtual void whenConnectFailed(EasyIO::TCP::IConnection* con, const std::string& reason){}
    virtual void whenDisconnected(EasyIO::TCP::IConnection* con){}
    virtual void whenLoggedIn(){}
    virtual void whenLoginFailed(int errCode, const std::string& errString){}


    Cryptology::AES<EasyIO::ByteBuffer>* aes();

private:
    void _whenConnected(EasyIO::TCP::IConnection* con);
    void _whenConnectFailed(EasyIO::TCP::IConnection* con, const std::string& reason);
    void _whenDisconnected(EasyIO::TCP::IConnection* con);
    void _whenReceivedData(EasyIO::TCP::IConnection* con, EasyIO::ByteBuffer data);

    void handleResponseOfLogin(const unsigned char* data, size_t len);

protected:
    std::string m_host;
    std::string m_ip;
    unsigned short m_port;
    std::string m_publicKey;
    std::string m_account;
    std::string m_password;

    User::ID m_selfUserId = 0;
    User::ID m_partnerUserId = 0;
    std::string m_authString;
    EasyIO::TCP::IClientPtr m_client;

    bool m_loggedIn = false;
    bool m_loggingIn = false;

    std::unique_ptr<Cryptology::RSA<EasyIO::ByteBuffer>> m_rsa;
    std::unique_ptr<Cryptology::AES<EasyIO::ByteBuffer>> m_aes;

    EasyIO::ByteBuffer m_buffer;
};

#endif // VIEWERBASE_H
