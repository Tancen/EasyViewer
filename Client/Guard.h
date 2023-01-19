#ifndef GUARD_H
#define GUARD_H

#include <string>
#include <set>
#include "Global/Component/EasyIO/EasyIO.h"
#include "Global/Component/Cryptology/Cryptology.hpp"
#include "Global/Define.h"
#include <vector>

class Guard
{
public:
    class IAbility
    {
    public:
        virtual ~IAbility(){}
        void setEntity(Guard* guard);

        virtual int role() = 0;
        virtual bool handleCompleteData(unsigned tag, const unsigned char* data, size_t len ) = 0;

        virtual void handleLoggedinEvent() = 0;
        virtual void handleConnectionLostEvent() = 0;

    protected:
        Cryptology::RSA<EasyIO::ByteBuffer>* rsa();
        Cryptology::AES<EasyIO::ByteBuffer>* aes();
        EasyIO::TCP::IConnection* connection();
        User::ID selfUserId();
        bool isLoggedIn();

    protected:
        Guard* m_entity = nullptr;
    };

    friend class IAbility;

public:
    Guard(const std::string& host, const std::string& ip, unsigned short port, const std::string& publicKey,
              const std::string& account, const std::string& password);
    virtual ~Guard();

    virtual std::pair<bool, std::string> init();
    virtual int role();

    const std::string& host();
    const std::string& ip();
    unsigned short port();
    void login();
    void connect();
    bool connected();
    bool connecting();
    void disconnect();

    bool loggedIn();
    bool loggingIn();

    User::ID getUserId();

    void addAbility(std::unique_ptr<IAbility> ability);

protected:
    virtual void whenConnected(EasyIO::TCP::IConnection* con);
    virtual void whenConnectFailed(EasyIO::TCP::IConnection* con, const std::string& reason);
    virtual void whenDisconnected(EasyIO::TCP::IConnection* con);
    virtual void whenReceivedData(EasyIO::TCP::IConnection* con, EasyIO::ByteBuffer data);

    void send(EasyIO::ByteBuffer data);

    virtual bool handleCompleteData(unsigned tag, const unsigned char* data, size_t len );
    void handleResponseOfLogin(const unsigned char* data, size_t len);

    void error(int errCode, const std::string& errString);

public:
    std::function<void(Guard*)> onConnected;
    std::function<void(Guard*)> onConnectFailed;
    std::function<void(Guard*)> onLoggedIn;
    std::function<void(Guard*, int, const std::string&)> onLoginFailed;
    std::function<void(Guard*)> onDisconnected;

    std::function<void(Guard*, int, const std::string&)> onError;

protected:
    std::string m_host;
    std::string m_ip;
    unsigned short m_port;
    std::string m_publicKey;
    std::string m_account;
    std::string m_password;

    User::ID m_userId = 0;
    EasyIO::TCP::IClientPtr m_client;

    bool m_loggedIn = false;
    bool m_loggingIn = false;

    std::unique_ptr<Cryptology::RSA<EasyIO::ByteBuffer>> m_rsa;
    std::unique_ptr<Cryptology::AES<EasyIO::ByteBuffer>> m_aes;

    EasyIO::ByteBuffer m_buffer;

    int m_role;
    std::vector<std::unique_ptr<IAbility>> m_abilities;
};

#endif // GUARD_H
