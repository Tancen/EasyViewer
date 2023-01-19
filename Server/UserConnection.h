#ifndef USERCONNECTION_H
#define USERCONNECTION_H

#include "Global/Component/EasyIO/EasyIO.h"
#include "IConnection.h"
#include "Account.h"
#include "Global/Component/Cryptology/Cryptology.hpp"
#include <atomic>
#include <QDateTime>
#include <list>
#include "NetworkDataHandler/INetworkDataHandler.h"

class UserConnection;
typedef std::shared_ptr<UserConnection> UserConnectionPtr;

class UserConnection : public IConnection, public std::enable_shared_from_this<UserConnection>
{
public:
    struct Request
    {
    public:
        Request(){}
        Request(UserConnectionPtr con, long long arrival, uint32_t tag, EasyIO::ByteBuffer data, NetworkDataHandler::INetworkDataHandlerPtr handler)
            : con(con), arrival(arrival), tag(tag), data(data), handler(handler)
        {

        }

        UserConnectionPtr con;
        long long arrival;
        uint32_t tag;
        EasyIO::ByteBuffer data;
        NetworkDataHandler::INetworkDataHandlerPtr handler;
    };

    typedef std::function<NetworkDataHandler::INetworkDataHandlerPtr(uint32_t tag)> ReceivedDataHandlerFactory;

public:
    static std::pair<UserConnectionPtr, std::string> create(EasyIO::TCP::IConnectionPtr con, const std::string& privateKey,
                                                            ReceivedDataHandlerFactory receivedDataHandlerFactory);

    const QDateTime& creatingTime();

    void setFirstCryptology(std::unique_ptr<Cryptology::RSA<EasyIO::ByteBuffer>> rsa);
    void setSecondCryptology(std::unique_ptr<Cryptology::AES<EasyIO::ByteBuffer>> aes);

    Cryptology::RSA<EasyIO::ByteBuffer>* getFirstCryptology();
    Cryptology::AES<EasyIO::ByteBuffer>* getSecondCryptology();

    const std::string& peerIP();
    unsigned short peerPort();

    bool bindUser(const Account &user, int role);
    const Account* getBindingUser();

    User::ID getTargetUserId();
    bool setTargetUserId(User::ID id);

    int getRole();

    void disconnect();
    void send(const unsigned char* data, size_t len);
    void send(EasyIO::ByteBuffer data);

    void handleReceivedData(const unsigned char* data, size_t len);

    bool isRequestsEmpty();
    bool popRequest(Request& request);
    std::pair<bool, size_t> pushRequest(const UserConnection::Request& r);

private:
    UserConnection(EasyIO::TCP::IConnectionPtr con, ReceivedDataHandlerFactory receivedDataHandlerFactory);

    void updateRLW(long t, double cost);

public:
    double reservation = 0; // in ms
    double proportion = 0;  // in ms
    double limit = 0;       // in ms

    int R = 50 * 1024;      // 50 KB/s
    int L = 1024 * 1024 * 2;    // 2 MB/s
    int W = 1;

    typedef int TAG_TYPE;
    static const TAG_TYPE NO_TAG = 0;
    TAG_TYPE tag = NO_TAG;

private:
    QDateTime m_creatingTime;

    EasyIO::TCP::IConnectionPtr m_con;
    EasyIO::ByteBuffer m_buffer;

    Account m_user;
    Account* m_pUser = nullptr;

    User::ID m_targetUserId = 0;
    int m_role = 0;

    std::unique_ptr<Cryptology::RSA<EasyIO::ByteBuffer>> m_rsa;
    std::unique_ptr<Cryptology::AES<EasyIO::ByteBuffer>> m_aes;

    std::list<Request> m_requests;
    std::recursive_mutex m_mutex;
    ReceivedDataHandlerFactory m_receivedDataHandlerFactory;
    size_t m_numPendingBytes = 0;
};

#endif // USERCONNECTION_H
