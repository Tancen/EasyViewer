#include "Client.h"
#include <QDateTime>
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Management/Server/Management.pb.h"
#include "Global/Protocol/User/User.pb.h"

bool Client::IResult::match(int64_t seq)
{
    return this->seq == seq;
}

bool Client::LoginResult::match(int64_t seq)
{
    return true;
}

Client::Promise::Promise(int64_t seq)
    :   m_seq(seq)
{

}

void Client::Promise::setValue(std::unique_ptr<IResult> result)
{
    if (result->match(m_seq))
        this->m_promise.set_value(std::move(result));
}

std::future<std::unique_ptr<Client::IResult> > Client::Promise::getFuture()
{
    return this->m_promise.get_future();
}

Client::Client(const std::string& ip, unsigned short port, const std::string& publicKey)
    :   m_ip(ip), m_port(port), m_publicKey(publicKey)
{

}

Client::~Client()
{

}

std::pair<bool, std::string> Client::init()
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

        m_client = EasyIO::TCP::Client::create();
        m_client->onConnected = std::bind(&Client::whenConnected, this, std::placeholders::_1);
        m_client->onConnectFailed = std::bind(&Client::whenConnectFailed, this, std::placeholders::_1, std::placeholders::_2);
        m_client->onDisconnected = std::bind(&Client::whenDisconnected, this, std::placeholders::_1);
        m_client->onBufferReceived = std::bind(&Client::whenReceivedData, this, std::placeholders::_1, std::placeholders::_2);

        srand(QDateTime::currentMSecsSinceEpoch());
        m_seq = rand();
        return std::make_pair(true, "");
    } while(0);

    return std::make_pair(false, errorString);
}

void Client::connect()
{
    m_client->connect(m_ip, m_port);
}

std::future<std::unique_ptr<Client::IResult>> Client::login(const std::string& authString)
{   
    int64_t seq = nextSEQ();
    auto ret = promise(seq);

    Global::Protocol::User::RequestLogin request;
    request.set_role(GLOBAL_CONNECTION_ROLE_MANAGER);
    request.set_password(authString);
    request.set_secret_key(m_aes->key());

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LOGIN, request, data, m_rsa)
    m_client->send(data);

    return ret;
}

std::future<std::unique_ptr<Client::IResult>> Client::listAccount(const std::string& filter)
{
    int64_t seq = nextSEQ();
    auto ret = promise(seq);

    Global::Protocol::Server::Management::RequestListAccount request;
    request.set_seq(seq);
    request.set_user_name_filter(filter);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_ACCOUNT, request, data, m_aes)
    m_client->send(data);

    return ret;
}

std::future<std::unique_ptr<Client::IResult>> Client::addAccount(const std::string& account, const std::string& password)
{
    int64_t seq = nextSEQ();
    auto ret = promise(seq);

    Global::Protocol::Server::Management::RequestAddAccount request;
    request.set_seq(seq);
    request.set_account(account);
    request.set_password(password);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ADD_ACCOUNT, request, data, m_aes)
    m_client->send(data);

    return ret;
}

std::future<std::unique_ptr<Client::IResult>> Client::enableAccount(const std::string& account)
{
    int64_t seq = nextSEQ();
    auto ret = promise(seq);

    Global::Protocol::Server::Management::RequestEnableAccount request;
    request.set_seq(seq);
    request.set_account(account);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ENABLE_ACCOUNT, request, data, m_aes)
    m_client->send(data);

    return ret;
}

std::future<std::unique_ptr<Client::IResult>> Client::disableAccount(const std::string& account)
{
    int64_t seq = nextSEQ();
    auto ret = promise(seq);

    Global::Protocol::Server::Management::RequestDisableAccount request;
    request.set_seq(seq);
    request.set_account(account);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_DISABLE_ACCOUNT, request, data, m_aes)
    m_client->send(data);

    return ret;
}

std::future<std::unique_ptr<Client::IResult>> Client::deleteAccount(const std::string& account)
{
    int64_t seq = nextSEQ();
    auto ret = promise(seq);

    Global::Protocol::Server::Management::RequestDeleteAccount request;
    request.set_seq(seq);
    request.set_account(account);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_DELETE_ACCOUNT, request, data, m_aes)
    m_client->send(data);

    return ret;
}


std::future<std::unique_ptr<Client::IResult>> Client::changeAccountPassword(const std::string& account, const std::string& newPassword)
{
    int64_t seq = nextSEQ();
    auto ret = promise(seq);

    Global::Protocol::Server::Management::RequestChangeAccountPassword request;
    request.set_seq(seq);
    request.set_account(account);
    request.set_password(newPassword);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_CHANGE_ACCOUNT_PASSWORD, request, data, m_aes)
    m_client->send(data);

    return ret;
}

std::future<std::unique_ptr<Client::IResult> > Client::listBlockedIPAddresses()
{
    int64_t seq = nextSEQ();
    auto ret = promise(seq);

    Global::Protocol::Server::Management::RequestListBlockedIPAddresses request;
    request.set_seq(seq);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_BLOCKED_IP_ADDRESSES, request, data, m_aes)
    m_client->send(data);

    return ret;
}

std::future<std::unique_ptr<Client::IResult> > Client::removeBlockedIPAddress(const std::string &ip)
{
    int64_t seq = nextSEQ();
    auto ret = promise(seq);

    Global::Protocol::Server::Management::RequestRemoveBlockedIPAddress request;
    request.set_seq(seq);
    request.set_ip_address(ip);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_REMOVE_BLOCKED_IP_ADDRESS, request, data, m_aes)
    m_client->send(data);

    return ret;
}

std::future<std::unique_ptr<Client::IResult> > Client::addBlockedIPAddress(const std::string &ip, unsigned duration)
{
    int64_t seq = nextSEQ();
    auto ret = promise(seq);

    Global::Protocol::Server::Management::RequestAddBlockedIPAddress request;
    request.set_seq(seq);
    request.set_ip_address(ip);
    request.set_duration(duration);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ADD_BLOCKED_IP_ADDRESS, request, data, m_aes)
    m_client->send(data);

    return ret;
}

std::future<std::unique_ptr<Client::IResult> > Client::listBlockedAccountss()
{
    int64_t seq = nextSEQ();
    auto ret = promise(seq);

    Global::Protocol::Server::Management::RequestListBlockedAccounts request;
    request.set_seq(seq);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_BLOCKED_ACCOUNTS, request, data, m_aes)
    m_client->send(data);

    return ret;
}

std::future<std::unique_ptr<Client::IResult> > Client::removeBlockedAccount(User::ID userId)
{
    int64_t seq = nextSEQ();
    auto ret = promise(seq);

    Global::Protocol::Server::Management::RequestRemoveBlockedAccount request;
    request.set_seq(seq);
    request.set_user_id(userId);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_REMOVE_BLOCKED_ACCOUNT, request, data, m_aes)
    m_client->send(data);

    return ret;
}

std::future<std::unique_ptr<Client::IResult> > Client::addBlockedAccount(User::ID userId, unsigned duration)
{
    int64_t seq = nextSEQ();
    auto ret = promise(seq);

    Global::Protocol::Server::Management::RequestAddBlockedAccount request;
    request.set_seq(seq);
    request.set_user_id(userId);
    request.set_duration(duration);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ADD_BLOCKED_ACCOUNT, request, data, m_aes)
    m_client->send(data);

    return ret;
}

std::future<std::unique_ptr<Client::IResult>> Client::promise(int64_t seq)
{
    std::lock_guard guard(m_mutex);
    m_promise.reset(new Promise(seq));
    return m_promise->getFuture();
}


void Client::whenConnected(EasyIO::TCP::IConnection *con)
{
    con->enableKeepalive(5000, 10000);
    con->recv(EasyIO::ByteBuffer());

    if (this->onConnected)
        this->onConnected();
}

void Client::whenConnectFailed(EasyIO::TCP::IConnection *con, const std::string& reason)
{
    if (this->onConnectFailed)
        this->onConnectFailed(reason);
}

void Client::whenDisconnected(EasyIO::TCP::IConnection *con)
{
    if (this->onDisconnected)
        this->onDisconnected();
}


void Client::whenReceivedData(EasyIO::TCP::IConnection* con, EasyIO::ByteBuffer data)
{
    m_buffer.write(data);

    for (;;)
    {
        if (m_buffer.numReadableBytes() < 8)
            break;

        uint32_t t = *(uint32_t*)m_buffer.readableBytes();
        uint32_t l =  *(uint32_t*)(m_buffer.readableBytes() + 4);

        static const size_t MAX_LEN = 1024 * 1024 * 10; //test
        if (l > MAX_LEN)
        {
            std::cout << "error: network packet too long" << std::endl;
            con->disconnect();
            break;
        }

        if (m_buffer.numReadableBytes() - 8 < l)
            break;

        if (!handleCompleteData(t, (const unsigned char*)m_buffer.readableBytes() + 8, l))
        {
            std::cout << "error: unknow tag %d" << std::endl;
            con->disconnect();
            break;
        }

        m_buffer.moveReaderIndex(l + 8);
    }

    data.clear();
    con->recv(data);
}

bool Client::handleCompleteData(unsigned tag, const unsigned char *data, size_t len)
{
    switch (tag)
    {
    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LOGIN:
        handleResponseOfLogin(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LIST_ACCOUNT:
        handleResponseOfListAccount(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_ADD_ACCOUNT:
        handleResponseOfAddtAccount(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_ENABLE_ACCOUNT:
        handleResponseOfEnableAccount(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_DISABLE_ACCOUNT:
        handleResponseOfDisableAccount(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_DELETE_ACCOUNT:
        handleResponseOfDeleteAccount(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_CHANGE_ACCOUNT_PASSWORD:
        handleResponseOfChangeAccountPassword(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LIST_BLOCKED_IP_ADDRESSES:
        handleResponseOfListBlockedIPAddresses(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_REMOVE_BLOCKED_IP_ADDRESS:
        handleResponseOfRemoveBlockedIPAddress(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_ADD_BLOCKED_IP_ADDRESS:
        handleResponseOfAddBlockedIPAddress(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LIST_BLOCKED_ACCOUNTS:
        handleResponseOfListBlockedAccounts(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_REMOVE_BLOCKED_ACCOUNT:
        handleResponseOfRemoveBlockedAccount(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_ADD_BLOCKED_ACCOUNT:
        handleResponseOfAddBlockedAccount(data, len);
        return true;
    }
    return false;
}

void Client::handleResponseOfLogin(const unsigned char* data, size_t len)
{
    auto data1 = m_rsa->decrypt((const char*)data, len);
    if (!data1.isOk)
    {
        std::cout << "error: decrypt data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    Global::Protocol::User::ResponseLogin response;
    if(!response.ParseFromArray(data1.data.readableBytes(), data1.data.numReadableBytes()))
    {
        std::cout << "error: unserialize data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    std::shared_ptr<Promise> promise = getPromising();
    if (promise.get())
    {
        LoginResult* result = new LoginResult();
        result->errCode = response.err_code();
        result->errString = response.err_string();
        promise->setValue(std::unique_ptr<IResult>(result));
    }
}

void Client::handleResponseOfListAccount(const unsigned char* data, size_t len)
{
    auto data1 = m_aes->decrypt((const char*)data, len);
    if (!data1.isOk)
    {
        std::cout << "error: decrypt data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    Global::Protocol::Server::Management::ResponseListAccount response;
    if(!response.ParseFromArray(data1.data.readableBytes(), data1.data.numReadableBytes()))
    {
        std::cout << "error: unserialize data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    std::shared_ptr<Promise> promise = getPromising();
    if (promise.get())
    {
        ListAccountResult* result = new ListAccountResult();
        result->seq = response.seq();
        result->errCode = response.err_code();
        result->errString = response.err_string();
        for (int i = 0; i < response.accounts_size(); i++)
        {
            auto& it = response.accounts(i);
            result->accounts.push_back(Account{it.user_id(), it.account(), it.password(), it.disabled()});
        }
        promise->setValue(std::unique_ptr<IResult>(result));
    }
}

void Client::handleResponseOfAddtAccount(const unsigned char* data, size_t len)
{
    auto data1 = m_aes->decrypt((const char*)data, len);
    if (!data1.isOk)
    {
        std::cout << "error: decrypt data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    Global::Protocol::Server::Management::ResponseAddAccount response;
    if(!response.ParseFromArray(data1.data.readableBytes(), data1.data.numReadableBytes()))
    {
        std::cout << "error: unserialize data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    std::shared_ptr<Promise> promise = getPromising();
    if (promise.get())
    {
        AddAccountResult* result = new AddAccountResult();
        result->seq = response.seq();
        result->errCode = response.err_code();
        result->errString = response.err_string();
        result->userId = response.user_id();
        promise->setValue(std::unique_ptr<IResult>(result));
    }
}

void Client::handleResponseOfEnableAccount(const unsigned char* data, size_t len)
{
    auto data1 = m_aes->decrypt((const char*)data, len);
    if (!data1.isOk)
    {
        std::cout << "error: decrypt data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    Global::Protocol::Server::Management::ResponseEnableAccount response;
    if(!response.ParseFromArray(data1.data.readableBytes(), data1.data.numReadableBytes()))
    {
        std::cout << "error: unserialize data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    std::shared_ptr<Promise> promise = getPromising();
    if (promise.get())
    {
        EnableAccountResult* result = new EnableAccountResult();
        result->seq = response.seq();
        result->errCode = response.err_code();
        result->errString = response.err_string();
        promise->setValue(std::unique_ptr<IResult>(result));
    }
}

void Client::handleResponseOfDisableAccount(const unsigned char* data, size_t len)
{
    auto data1 = m_aes->decrypt((const char*)data, len);
    if (!data1.isOk)
    {
        std::cout << "error: decrypt data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    Global::Protocol::Server::Management::ResponseDisableAccount response;
    if(!response.ParseFromArray(data1.data.readableBytes(), data1.data.numReadableBytes()))
    {
        std::cout << "error: unserialize data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    std::shared_ptr<Promise> promise = getPromising();
    if (promise.get())
    {
        DisableAccountResult* result = new DisableAccountResult();
        result->seq = response.seq();
        result->errCode = response.err_code();
        result->errString = response.err_string();
        promise->setValue(std::unique_ptr<IResult>(result));
    }
}


void Client::handleResponseOfDeleteAccount(const unsigned char* data, size_t len)
{
    auto data1 = m_aes->decrypt((const char*)data, len);
    if (!data1.isOk)
    {
        std::cout << "error: decrypt data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    Global::Protocol::Server::Management::ResponseDeleteAccount response;
    if(!response.ParseFromArray(data1.data.readableBytes(), data1.data.numReadableBytes()))
    {
        std::cout << "error: unserialize data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    std::shared_ptr<Promise> promise = getPromising();
    if (promise.get())
    {
        DeleteAccountResult* result = new DeleteAccountResult();
        result->seq = response.seq();
        result->errCode = response.err_code();
        result->errString = response.err_string();
        promise->setValue(std::unique_ptr<IResult>(result));
    }
}

void Client::handleResponseOfChangeAccountPassword(const unsigned char* data, size_t len)
{
    auto data1 = m_aes->decrypt((const char*)data, len);
    if (!data1.isOk)
    {
        std::cout << "error: decrypt data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    Global::Protocol::Server::Management::ResponseChangeAccountPassword response;
    if(!response.ParseFromArray(data1.data.readableBytes(), data1.data.numReadableBytes()))
    {
        std::cout << "error: unserialize data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    std::shared_ptr<Promise> promise = getPromising();
    if (promise.get())
    {
        ChangeAccountPasswordResult* result = new ChangeAccountPasswordResult();
        result->seq = response.seq();
        result->errCode = response.err_code();
        result->errString = response.err_string();
        promise->setValue(std::unique_ptr<IResult>(result));
    }
}

void Client::handleResponseOfListBlockedIPAddresses(const unsigned char *data, size_t len)
{
    auto data1 = m_aes->decrypt((const char*)data, len);
    if (!data1.isOk)
    {
        std::cout << "error: decrypt data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    Global::Protocol::Server::Management::ResponseListBlockedIPAddresses response;
    if(!response.ParseFromArray(data1.data.readableBytes(), data1.data.numReadableBytes()))
    {
        std::cout << "error: unserialize data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    std::shared_ptr<Promise> promise = getPromising();
    if (promise.get())
    {
        ListBlockedIPAddressesResult* result = new ListBlockedIPAddressesResult();
        result->seq = response.seq();
        result->errCode = response.err_code();
        result->errString = response.err_string();
        for (int i = 0; i < response.ip_addresses_size(); i++)
        {
            auto& addr = response.ip_addresses(i);
            result->ipAddresses.push_back({addr.ip_address(), addr.until()});
        }
        promise->setValue(std::unique_ptr<IResult>(result));
    }
}

void Client::handleResponseOfRemoveBlockedIPAddress(const unsigned char *data, size_t len)
{
    auto data1 = m_aes->decrypt((const char*)data, len);
    if (!data1.isOk)
    {
        std::cout << "error: decrypt data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    Global::Protocol::Server::Management::ResponseRemoveBlockedIPAddress response;
    if(!response.ParseFromArray(data1.data.readableBytes(), data1.data.numReadableBytes()))
    {
        std::cout << "error: unserialize data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    std::shared_ptr<Promise> promise = getPromising();
    if (promise.get())
    {
        RemoveBlockedIPAddressResult* result = new RemoveBlockedIPAddressResult();
        result->seq = response.seq();
        result->errCode = response.err_code();
        result->errString = response.err_string();
        promise->setValue(std::unique_ptr<IResult>(result));
    }
}

void Client::handleResponseOfAddBlockedIPAddress(const unsigned char *data, size_t len)
{
    auto data1 = m_aes->decrypt((const char*)data, len);
    if (!data1.isOk)
    {
        std::cout << "error: decrypt data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    Global::Protocol::Server::Management::ResponseAddBlockedIPAddress response;
    if(!response.ParseFromArray(data1.data.readableBytes(), data1.data.numReadableBytes()))
    {
        std::cout << "error: unserialize data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    std::shared_ptr<Promise> promise = getPromising();
    if (promise.get())
    {
        AddBlockedIPAddressResult* result = new AddBlockedIPAddressResult();
        result->seq = response.seq();
        result->errCode = response.err_code();
        result->errString = response.err_string();
        promise->setValue(std::unique_ptr<IResult>(result));
    }
}

void Client::handleResponseOfListBlockedAccounts(const unsigned char *data, size_t len)
{
    auto data1 = m_aes->decrypt((const char*)data, len);
    if (!data1.isOk)
    {
        std::cout << "error: decrypt data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    Global::Protocol::Server::Management::ResponseListBlockedAccounts response;
    if(!response.ParseFromArray(data1.data.readableBytes(), data1.data.numReadableBytes()))
    {
        std::cout << "error: unserialize data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    std::shared_ptr<Promise> promise = getPromising();
    if (promise.get())
    {
        ListBlockedAccountsResult* result = new ListBlockedAccountsResult();
        result->seq = response.seq();
        result->errCode = response.err_code();
        result->errString = response.err_string();
        for (int i = 0; i < response.accounts_size(); i++)
        {
            auto& account = response.accounts(i);
            result->accounts.push_back({account.user_id(), account.account(), account.until()});
        }
        promise->setValue(std::unique_ptr<IResult>(result));
    }
}

void Client::handleResponseOfRemoveBlockedAccount(const unsigned char *data, size_t len)
{
    auto data1 = m_aes->decrypt((const char*)data, len);
    if (!data1.isOk)
    {
        std::cout << "error: decrypt data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    Global::Protocol::Server::Management::ResponseRemoveBlockedAccount response;
    if(!response.ParseFromArray(data1.data.readableBytes(), data1.data.numReadableBytes()))
    {
        std::cout << "error: unserialize data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    std::shared_ptr<Promise> promise = getPromising();
    if (promise.get())
    {
        RemoveBlockedAccountResult* result = new RemoveBlockedAccountResult();
        result->seq = response.seq();
        result->errCode = response.err_code();
        result->errString = response.err_string();
        promise->setValue(std::unique_ptr<IResult>(result));
    }
}

void Client::handleResponseOfAddBlockedAccount(const unsigned char *data, size_t len)
{
    auto data1 = m_aes->decrypt((const char*)data, len);
    if (!data1.isOk)
    {
        std::cout << "error: decrypt data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    Global::Protocol::Server::Management::ResponseAddBlockedAccount response;
    if(!response.ParseFromArray(data1.data.readableBytes(), data1.data.numReadableBytes()))
    {
        std::cout << "error: unserialize data failed" << std::endl;
        m_client->disconnect();
        return;
    }

    std::shared_ptr<Promise> promise = getPromising();
    if (promise.get())
    {
        AddBlockedAccountResult* result = new AddBlockedAccountResult();
        result->seq = response.seq();
        result->errCode = response.err_code();
        result->errString = response.err_string();
        promise->setValue(std::unique_ptr<IResult>(result));
    }
}

std::shared_ptr<Client::Promise> Client::getPromising()
{
    std::shared_ptr<Promise> promise;
    m_mutex.lock();
    promise = m_promise;
    m_mutex.unlock();
    return promise;
}

int64_t Client::nextSEQ()
{
    return ++m_seq;
}
