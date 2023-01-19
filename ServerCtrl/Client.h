#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <future>
#include "Global/Component/EasyIO/EasyIO.h"
#include "Global/Component/Cryptology/Cryptology.hpp"
#include "Global/Define.h"
#include "Structs.h"

class Client
{
public:
    struct IResult
    {
        int64_t seq;
        int errCode;
        std::string errString;

        virtual bool match(int64_t seq);
    };
    // -- login
    struct LoginResult : public IResult
    {
        bool match(int64_t seq) override;
    };

    // -- account
    struct ListAccountResult : public IResult
    {
        std::vector<Account> accounts;
    };

    struct AddAccountResult : public IResult
    {
        User::ID userId;
    };

    struct EnableAccountResult : public IResult
    {
    };

    struct DisableAccountResult : public IResult
    {
    };

    struct DeleteAccountResult : public IResult
    {
    };

    struct ChangeAccountPasswordResult : public IResult
    {
    };
    // -- blacklist
    struct ListBlockedIPAddressesResult : public IResult
    {
        std::vector<BlockedIPAddress> ipAddresses;
    };

    struct RemoveBlockedIPAddressResult : public IResult
    {
    };

    struct AddBlockedIPAddressResult : public IResult
    {
    };

    struct ListBlockedAccountsResult : public IResult
    {
        std::vector<BlockedAccount> accounts;
    };

    struct RemoveBlockedAccountResult : public IResult
    {
    };

    struct AddBlockedAccountResult : public IResult
    {
    };

    class Promise
    {
    public:
        Promise(int64_t seq);

        void setValue(std::unique_ptr<IResult> result);
        std::future<std::unique_ptr<IResult>> getFuture();

    private:
        int64_t m_seq;
        std::promise<std::unique_ptr<IResult>> m_promise;
    };

public:
    Client(const std::string& ip, unsigned short port, const std::string& publicKey);
    ~Client();

    std::pair<bool, std::string> init();

    void connect();

    std::future<std::unique_ptr<IResult>> login(const std::string& authString);

    std::future<std::unique_ptr<IResult>> listAccount(const std::string& filter);
    std::future<std::unique_ptr<IResult>> addAccount(const std::string& account, const std::string& password);
    std::future<std::unique_ptr<IResult>> enableAccount(const std::string& account);
    std::future<std::unique_ptr<IResult>> disableAccount(const std::string& account);
    std::future<std::unique_ptr<IResult>> deleteAccount(const std::string& account);
    std::future<std::unique_ptr<IResult>> changeAccountPassword(
            const std::string& account, const std::string& newPassword);

    std::future<std::unique_ptr<IResult>> listBlockedIPAddresses();
    std::future<std::unique_ptr<IResult>> removeBlockedIPAddress(const std::string& ip);
    std::future<std::unique_ptr<IResult>> addBlockedIPAddress(const std::string& ip, unsigned duration);
    std::future<std::unique_ptr<IResult>> listBlockedAccountss();
    std::future<std::unique_ptr<IResult>> removeBlockedAccount(User::ID userId);
    std::future<std::unique_ptr<IResult>> addBlockedAccount(User::ID userId, unsigned duration);

private:
    void whenConnected(EasyIO::TCP::IConnection* con);
    void whenConnectFailed(EasyIO::TCP::IConnection* con, const std::string& reason);
    void whenDisconnected(EasyIO::TCP::IConnection* con);
    void whenReceivedData(EasyIO::TCP::IConnection* con, EasyIO::ByteBuffer data);

    bool handleCompleteData(unsigned tag, const unsigned char* data, size_t len );

    void handleResponseOfLogin(const unsigned char* data, size_t len);
    void handleResponseOfListAccount(const unsigned char* data, size_t len);
    void handleResponseOfAddtAccount(const unsigned char* data, size_t len);
    void handleResponseOfEnableAccount(const unsigned char* data, size_t len);
    void handleResponseOfDisableAccount(const unsigned char* data, size_t len);
    void handleResponseOfDeleteAccount(const unsigned char* data, size_t len);
    void handleResponseOfChangeAccountPassword(const unsigned char* data, size_t len);

    void handleResponseOfListBlockedIPAddresses(const unsigned char* data, size_t len);
    void handleResponseOfRemoveBlockedIPAddress(const unsigned char* data, size_t len);
    void handleResponseOfAddBlockedIPAddress(const unsigned char* data, size_t len);
    void handleResponseOfListBlockedAccounts(const unsigned char* data, size_t len);
    void handleResponseOfRemoveBlockedAccount(const unsigned char* data, size_t len);
    void handleResponseOfAddBlockedAccount(const unsigned char* data, size_t len);

    std::future<std::unique_ptr<IResult>> promise(int64_t seq);

    int64_t nextSEQ();

    std::shared_ptr<Promise> getPromising();

public:
    std::function<void()> onConnected;
    std::function<void(const std::string&)> onConnectFailed;
    std::function<void()> onDisconnected;

private:
    std::string m_ip;
    unsigned short m_port;
    std::string m_publicKey;

    EasyIO::TCP::IClientPtr m_client;

    std::unique_ptr<Cryptology::RSA<EasyIO::ByteBuffer>> m_rsa;
    std::unique_ptr<Cryptology::AES<EasyIO::ByteBuffer>> m_aes;

    std::recursive_mutex m_mutex;
    std::shared_ptr<Promise> m_promise;

    EasyIO::ByteBuffer m_buffer;
    int64_t m_seq;
};

#endif // CLIENT_H
