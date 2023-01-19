#include "Guard.h"
#include "Global/Protocol/Error.h"
#include "Global/Protocol/User/User.pb.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Define.h"
#include <qbytearray.h>
#include "UsualParseProtobufDataMacro.h"

Guard::Guard(const std::string &host, const std::string &ip, unsigned short port,
    const std::string& publicKey,
    const std::string &account, const std::string &password)
    :   m_host(host), m_ip(ip), m_port(port), m_publicKey(publicKey), m_account(account), m_password(password),
        m_client(EasyIO::TCP::Client::create()),
        m_role(GLOBAL_CONNECTION_ROLE_FLAG_GUARD)
{
    m_client->onConnected = std::bind(&Guard::whenConnected, this, std::placeholders::_1);
    m_client->onConnectFailed = std::bind(&Guard::whenConnectFailed, this, std::placeholders::_1, std::placeholders::_2);
    m_client->onDisconnected = std::bind(&Guard::whenDisconnected, this, std::placeholders::_1);
    m_client->onBufferReceived = std::bind(&Guard::whenReceivedData, this, std::placeholders::_1, std::placeholders::_2);
}

Guard::~Guard()
{
    m_client->syncDisconnect();
    m_client.reset();
}

std::pair<bool, std::string> Guard::init()
{
    std::string errorString;
    do
    {
        m_rsa = Cryptology::RSA<EasyIO::ByteBuffer>::newInstanceForPublicKey(m_publicKey, &errorString);
        if (!m_rsa)
        {
            errorString = "init RSA instance with public key: \n" + m_publicKey + " \n failed: " + errorString;
            break;
        }

        m_aes = Cryptology::AES<EasyIO::ByteBuffer>::newInstance(&errorString);
        if (!m_aes)
        {
            errorString = "init AES instance failed: " + errorString;
            break;
        }

        return std::make_pair(true, "");
    } while(0);

    return std::make_pair(false, errorString);
}

int Guard::role()
{
    return m_role;
}

const std::string &Guard::host()
{
    return m_host;
}

const std::string &Guard::ip()
{
    return m_ip;
}

unsigned short Guard::port()
{
    return m_port;
}

void Guard::login()
{
    if (m_loggingIn || m_loggedIn)
        return;
    m_loggingIn = true;

    EasyIO::ByteBuffer data;
    Global::Protocol::User::RequestLogin request;
    request.set_account(m_account);
    request.set_password(m_password);
    request.set_role(role());
    request.set_secret_key(m_aes->key());
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LOGIN, request, data, m_rsa)

    send(data);
}

void Guard::connect()
{
    if (m_client->connected())
        return;

    m_client->connect(m_ip, m_port);
}

bool Guard::connected()
{
    return m_client->connected();
}

bool Guard::connecting()
{
    return m_client->connecting();
}

void Guard::disconnect()
{
    m_client->disconnect();
}

bool Guard::loggedIn()
{
    return m_loggedIn;
}

bool Guard::loggingIn()
{
    return m_loggingIn;
}

User::ID Guard::getUserId()
{
    return m_userId;
}

void Guard::addAbility(std::unique_ptr<IAbility> ability)
{
    assert(!(m_role & ability->role()));

    ability->setEntity(this);
    m_role |= ability->role();
    m_abilities.push_back(std::move(ability));
}

void Guard::whenConnected(EasyIO::TCP::IConnection *con)
{
    Logger::info("%s:%d - %s:%d connected",  __PRETTY_FUNCTION__, __LINE__, con->peerIP().c_str(), con->peerPort());
    con->enableKeepalive(5000, 10000);
    con->setLinger(1, 0);
    if (onConnected)
        onConnected(this);

    con->recv(EasyIO::ByteBuffer());
}

void Guard::whenConnectFailed(EasyIO::TCP::IConnection *con, const std::string& reason)
{
    m_loggingIn = false;
    Logger::info("%s:%d - %s:%d connect failed[%s]",  __PRETTY_FUNCTION__, __LINE__, con->peerIP().c_str(), con->peerPort(), reason.c_str());

    if (onConnectFailed)
        onConnectFailed(this);
}

void Guard::whenDisconnected(EasyIO::TCP::IConnection *con)
{
    Logger::info("%s:%d - %s:%d disconnected",  __PRETTY_FUNCTION__, __LINE__, con->peerIP().c_str(), con->peerPort());

    m_loggedIn = false;
    m_loggingIn = false;

    for (auto& ability : m_abilities)
    {
        ability->handleConnectionLostEvent();
    }

    if (onDisconnected)
        onDisconnected(this);
}

void Guard::whenReceivedData(EasyIO::TCP::IConnection *con, EasyIO::ByteBuffer data)
{
    m_buffer.write(data);

    for (;;)
    {
        if (m_buffer.numReadableBytes() < 8)
            break;

        uint32_t t = *(uint32_t*)m_buffer.readableBytes();
        uint32_t l =  *(uint32_t*)(m_buffer.readableBytes() + 4);

        static const size_t MAX_LEN = 1024 * 1024;
        if (l > MAX_LEN)
        {
            Logger::warning("%s:%d - l >= MAX_LEN", __PRETTY_FUNCTION__, __LINE__);
            con->disconnect();
            break;
        }

        if (m_buffer.numReadableBytes() - 8 < l)
            break;

        switch (t)
        {
        case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LOGIN:
            handleResponseOfLogin((const unsigned char*)m_buffer.readableBytes() + 8, l);
            break;

        default:
            if (!handleCompleteData(t, (const unsigned char*)m_buffer.readableBytes() + 8, l))
            {
                Logger::warning("%s:%d - l >= unknow tag %d", __PRETTY_FUNCTION__, __LINE__, (int)t);
                con->disconnect();
                break;
            }
        }
        m_buffer.moveReaderIndex(l + 8);
    }

    data.clear();
    con->recv(data);
}

void Guard::send(EasyIO::ByteBuffer data)
{
    m_client->send(data);
}

bool Guard::handleCompleteData(unsigned tag, const unsigned char *data, size_t len)
{
    for (auto& ability : m_abilities)
    {
        if (ability->handleCompleteData(tag, data, len))
            return true;
    }

    return false;
}

void Guard::handleResponseOfLogin(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::User::ResponseLogin, response, data, len, m_rsa)

    if (response.err_code() != GLOBAL_PROTOCOL_ERR_NO_ERROR)
    {
        m_loggingIn = false;
        if (response.err_code() != GLOBAL_PROTOCOL_ERR_REPEAT_LOGIN)
        {
            Logger::info("%s:%d - %s:%d login failed[%d]: %s",  __PRETTY_FUNCTION__, __LINE__, m_client->peerIP().c_str(), m_client->peerPort(),
                response.err_code(), response.err_string().c_str());

            if (onLoginFailed)
                onLoginFailed(this, response.err_code(), response.err_string());
        }
    }
    else
    {
        m_userId = response.user_id();
        m_loggedIn = true;
        m_loggingIn = false;
        for (auto& ability : m_abilities)
        {
            ability->handleLoggedinEvent();
        }
        if (onLoggedIn)
            onLoggedIn(this);
        Logger::info("%s:%d - %s:%d logged in",  __PRETTY_FUNCTION__, __LINE__, m_client->peerIP().c_str(), m_client->peerPort());
    }
}

void Guard::error(int errCode, const std::string &errString)
{
    if (onError)
        onError(this, errCode, errString);
}

void Guard::IAbility::setEntity(Guard *guard)
{
    this->m_entity = guard;
}

Cryptology::RSA<EasyIO::ByteBuffer> *Guard::IAbility::rsa()
{
    return m_entity->m_rsa.get();
}

Cryptology::AES<EasyIO::ByteBuffer>* Guard::IAbility::aes()
{
    return m_entity->m_aes.get();
}

EasyIO::TCP::IConnection *Guard::IAbility::connection()
{
    return m_entity->m_client.get();
}

User::ID Guard::IAbility::selfUserId()
{
    return m_entity->getUserId();
}

bool Guard::IAbility::isLoggedIn()
{
    return m_entity->loggedIn();
}
