#include "ViewerBase.h"

#include "Global/Protocol/Error.h"
#include "Global/Protocol/User/User.pb.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Define.h"
#include "UsualParseProtobufDataMacro.h"

ViewerBase::ViewerBase(const std::string& peerHost, const std::string& peerIP, unsigned short peerPort,
                       const std::string& publicKey,
                       const std::string& account, const std::string& password,
                       User::ID partnerUserId, const std::string& authString)
    :   m_host(peerHost), m_ip(peerIP), m_port(peerPort), m_publicKey(publicKey), m_account(account), m_password(password),
        m_partnerUserId(partnerUserId), m_authString(authString),
        m_client(EasyIO::TCP::Client::create())
{
    m_client->onConnected = std::bind(&ViewerBase::_whenConnected, this, std::placeholders::_1);
    m_client->onConnectFailed = std::bind(&ViewerBase::_whenConnectFailed, this, std::placeholders::_1, std::placeholders::_2);
    m_client->onDisconnected = std::bind(&ViewerBase::_whenDisconnected, this, std::placeholders::_1);
    m_client->onBufferReceived = std::bind(&ViewerBase::_whenReceivedData, this, std::placeholders::_1, std::placeholders::_2);
}

ViewerBase::~ViewerBase()
{
    m_client->syncDisconnect();
    m_client.reset();
}

std::pair<bool, std::string> ViewerBase::init()
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

void ViewerBase::login()
{
    if (m_loggingIn || m_loggedIn)
        return;

    whenConnecting(m_client.get());
    m_client->connect(m_ip, m_port);
}

bool ViewerBase::loggedIn()
{
    return m_loggedIn;
}

bool ViewerBase::loggingIn()
{
    return m_loggingIn;
}

const std::string &ViewerBase::peerHost()
{
    return m_host;
}

const std::string &ViewerBase::peerIP()
{
    return m_ip;
}

unsigned short ViewerBase::peerPort()
{
    return m_port;
}

User::ID ViewerBase::getSelfUserId()
{
    return m_selfUserId;
}

User::ID ViewerBase::getPartnerUserId()
{
    return m_partnerUserId;
}

const std::string &ViewerBase::getAuthString()
{
    return m_authString;
}

void ViewerBase::_whenConnected(EasyIO::TCP::IConnection *con)
{
    Logger::info("%s:%d - %s:%d connected",  __PRETTY_FUNCTION__, __LINE__, con->peerIP().c_str(), con->peerPort());
    con->enableKeepalive(5000, 10000);
    con->setLinger(1, 0);
    con->recv(EasyIO::ByteBuffer());

    whenConnected(con);
    {
        EasyIO::ByteBuffer data;
        Global::Protocol::User::RequestLogin request;
        request.set_account(m_account);
        request.set_password(m_password);
        request.set_role(role());
        request.set_secret_key(m_aes->key());
        GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LOGIN, request, data, m_rsa)
        con->send(data);
    }
}

void ViewerBase::_whenConnectFailed(EasyIO::TCP::IConnection *con, const std::string& reason)
{
    m_loggingIn = false;
    Logger::info("%s:%d - %s:%d connect failed[%s]",  __PRETTY_FUNCTION__, __LINE__, con->peerIP().c_str(), con->peerPort(), reason.c_str());

   whenConnectFailed(con, reason);
}

void ViewerBase::_whenDisconnected(EasyIO::TCP::IConnection *con)
{
    Logger::info("%s:%d - %s:%d disconnected",  __PRETTY_FUNCTION__, __LINE__, con->peerIP().c_str(), con->peerPort());

    m_loggedIn = false;
    m_loggingIn = false;
    whenDisconnected(con);
}

void ViewerBase::_whenReceivedData(EasyIO::TCP::IConnection *con, EasyIO::ByteBuffer data)
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

void ViewerBase::send(EasyIO::ByteBuffer data)
{
    m_client->send(data);
}

void ViewerBase::handleResponseOfLogin(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::User::ResponseLogin, response, data, len, m_rsa)

    m_loggingIn = false;
    if (response.err_code() != GLOBAL_PROTOCOL_ERR_NO_ERROR
            && response.err_code() != GLOBAL_PROTOCOL_ERR_REPEAT_LOGIN)
    {
        whenLoginFailed(response.err_code(), response.err_string());
    }
    else
    {
        m_selfUserId = response.user_id();
        m_loggedIn = true;
        Logger::info("%s:%d - %s:%d logged in",  __PRETTY_FUNCTION__, __LINE__, m_client->peerIP().c_str(), m_client->peerPort());
        whenLoggedIn();
    }
}

Cryptology::AES<EasyIO::ByteBuffer> *ViewerBase::aes()
{
    return m_aes.get();
}
