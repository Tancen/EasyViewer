#include "NetworkDataHandler/Factory.h"
#include "Global/Component/Logger/Logger.h"
#include <assert.h>
#include <algorithm>
#include "UserConnection.h"
#include "RequestProcessor.h"
#include "Global/Define.h"

#define MAX_PENDING_BYTES  10485760  // 10MB

UserConnection::UserConnection(EasyIO::TCP::IConnectionPtr con, ReceivedDataHandlerFactory receivedDataHandlerFactory)
    :   m_creatingTime(QDateTime::currentDateTime()),
        m_con(con),
        m_receivedDataHandlerFactory(receivedDataHandlerFactory)

{
    assert(con.get());
    assert(receivedDataHandlerFactory);
}

std::pair<bool, size_t> UserConnection::pushRequest(const UserConnection::Request& r)
{
    std::lock_guard g(m_mutex);
    size_t size = m_requests.size();

    if (m_numPendingBytes >= MAX_PENDING_BYTES)
    {
        return {false, size};
    }
    else
    {
        m_requests.push_back(r);
        m_numPendingBytes += r.data.numReadableBytes();
    }

    if (!size)
        updateRLW(r.arrival, r.data.numReadableBytes());

    return {true, size};
}

void UserConnection::updateRLW(long t, double cost)
{
    static const unsigned U = 1000;
    reservation += cost / R * U;
    limit += cost / L * U;
    proportion += cost / W * U;

    reservation = std::max((double)t - 500, reservation); // 500ms offset
    limit = std::max((double)t, limit);
    proportion = std::max((double)t, proportion);
}

std::pair<UserConnectionPtr, std::string> UserConnection::create(EasyIO::TCP::IConnectionPtr con, const std::string& privateKey,
                                                                 ReceivedDataHandlerFactory receivedDataHandlerFactory)
{
    assert(con.get());
    assert(receivedDataHandlerFactory);

    std::string errorString;
    std::unique_ptr<Cryptology::RSA<EasyIO::ByteBuffer>> rsa = Cryptology::RSA<EasyIO::ByteBuffer>::newInstanceForPrivateKey(privateKey, &errorString);
    if (!rsa.get())
    {
        return std::make_pair(UserConnectionPtr(), errorString);
    }
    auto con1 = UserConnectionPtr(new UserConnection(con, receivedDataHandlerFactory));
    con1->setFirstCryptology(std::move(rsa));

    return std::make_pair(con1, "");
}

const QDateTime &UserConnection::creatingTime()
{
    return m_creatingTime;
}

void UserConnection::setFirstCryptology(std::unique_ptr<Cryptology::RSA<EasyIO::ByteBuffer> > rsa)
{
    m_rsa = std::move(rsa);
}

void UserConnection::setSecondCryptology(std::unique_ptr<Cryptology::AES<EasyIO::ByteBuffer> > aes)
{
    m_aes = std::move(aes);
}

Cryptology::RSA<EasyIO::ByteBuffer> *UserConnection::getFirstCryptology()
{
    return m_rsa.get();
}

Cryptology::AES<EasyIO::ByteBuffer> *UserConnection::getSecondCryptology()
{
    return m_aes.get();
}

unsigned short UserConnection::peerPort()
{
    return m_con->peerPort();
}

const std::string& UserConnection::peerIP()
{
    return m_con->peerIP();
}

bool UserConnection::bindUser(const Account &user, int role)
{
    if (m_pUser)
        return false;
    m_user = user;
    m_pUser = &m_user;
    m_role = role;
    return true;
}

const Account *UserConnection::getBindingUser()
{
    return m_pUser;
}

User::ID UserConnection::getTargetUserId()
{
    return m_targetUserId;
}

bool UserConnection::setTargetUserId(User::ID id)
{
    if (m_targetUserId)
        return false;
    m_targetUserId = id;
    return true;
}

int UserConnection::getRole()
{
    return m_role;
}

void UserConnection::disconnect()
{
    m_con->disconnect();
}

void UserConnection::send(const unsigned char *data, size_t len)
{
    EasyIO::ByteBuffer buf((const char*)data, len);
    send(buf);
}

void UserConnection::send(EasyIO::ByteBuffer buf)
{
    m_con->send(buf);
    if (m_con->numBytesPending() >= MAX_PENDING_BYTES)
    {
        disconnect();
        return;
    }
}

void UserConnection::handleReceivedData(const unsigned char* data, size_t len)
{
    m_buffer.write(data, len);

    for (;;)
    {
        if (m_buffer.numReadableBytes() < 8)
            break;

        uint32_t t = *(uint32_t*)m_buffer.readableBytes();
        uint32_t l =  *(uint32_t*)(m_buffer.readableBytes() + 4);

        static const size_t MAX_LEN = 1024 * 1024;
        if (l > MAX_LEN)
        {
            Logger::warning("%s:%d - l[%u] >= MAX_LEN", __PRETTY_FUNCTION__, __LINE__, l);
            this->disconnect();
            break;
        }

        if (m_buffer.numReadableBytes() - 8 < l)
            break;

        auto handler = m_receivedDataHandlerFactory(t);
        if (!handler.get())
        {
            Logger::warning("%s:%d - unknow tag %d", __PRETTY_FUNCTION__, __LINE__, t);
            this->disconnect();
            return;
        }

        auto con = this->shared_from_this();
        if(!RequestProcessor::share()->push(con, t, EasyIO::ByteBuffer(m_buffer.readableBytes() + 8, l), handler))
            disconnect();

        m_buffer.moveReaderIndex(l + 8);
    }
}

bool UserConnection::isRequestsEmpty()
{
    std::lock_guard g(m_mutex);
    return m_requests.empty();
}

bool UserConnection::popRequest(UserConnection::Request& request)
{
    std::lock_guard g(m_mutex);
    if (m_requests.empty())
        return false;

    request = m_requests.front();
    m_requests.pop_front();
    m_numPendingBytes -= request.data.numReadableBytes();

    if (!m_requests.empty())
    {
        const Request& next = m_requests.front();
        updateRLW(next.arrival, next.data.numReadableBytes());
    }

    return true;
}
