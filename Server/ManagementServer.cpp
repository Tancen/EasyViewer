#include "ManagementServer.h"
#include <cinttypes>
#include "Global/Define.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Component/Logger/Logger.h"
#include "RequestProcessor.h"
#include "NetworkDataHandler/User/RequestLoginHandler.h"
#include "NetworkDataHandler/Management/RequestAddAccountHandler.h"
#include "NetworkDataHandler/Management/RequestChangeAccountPasswordHandler.h"
#include "NetworkDataHandler/Management/RequestDisableAccountHandler.h"
#include "NetworkDataHandler/Management/RequestEnableAccountHandler.h"
#include "NetworkDataHandler/Management/RequestListAccountHandler.h"
#include "NetworkDataHandler/Management/RequestDeleteAccountHandler.h"
#include "NetworkDataHandler/Management/RequestListBlockedIPAddressesHandler.h"
#include "NetworkDataHandler/Management/RequestRemoveBlockedIPAddressHandler.h"
#include "NetworkDataHandler/Management/RequestAddBlockedIPAddressHandler.h"
#include "NetworkDataHandler/Management/RequestListBlockedAccountsHandler.h"
#include "NetworkDataHandler/Management/RequestRemoveBlockedAccountHandler.h"
#include "NetworkDataHandler/Management/RequestAddBlockedAccountHandler.h"

#define TAG_CLEAN   0

ManagementServer ManagementServer::s_this;

ManagementServer *ManagementServer::share()
{
    return &s_this;
}

ManagementServer::ManagementServer()
{

}

ManagementServer::~ManagementServer()
{

}

void ManagementServer::whenConnected(UserConnectionPtr con)
{

}

void ManagementServer::whenDisconnected(UserConnectionPtr con)
{
    RequestProcessor::share()->push(con, TAG_CLEAN, EasyIO::ByteBuffer(),
                NetworkDataHandler::INetworkDataHandlerPtr(new CleanUserConnection()));
}

UserConnection::ReceivedDataHandlerFactory ManagementServer::getReceivedDataHandlerFactory()
{
    static NetworkDataHandler::INetworkDataHandlerPtr sRequestLoginHandler(new NetworkDataHandler::RequestLogin());
    static NetworkDataHandler::INetworkDataHandlerPtr sRequestAddAccountHandler(new NetworkDataHandler::RequestAddAccount());
    static NetworkDataHandler::INetworkDataHandlerPtr sRequestChangeAccountPasswordHandler(new NetworkDataHandler::RequestChangeAccountPassword());
    static NetworkDataHandler::INetworkDataHandlerPtr sRequestDisableAccountHandler(new NetworkDataHandler::RequestDisableAccount());
    static NetworkDataHandler::INetworkDataHandlerPtr sRequestEnableAccountHandler(new NetworkDataHandler::RequestEnableAccount());
    static NetworkDataHandler::INetworkDataHandlerPtr sRequestListAccountHandler(new NetworkDataHandler::RequestListAccount());
    static NetworkDataHandler::INetworkDataHandlerPtr sRequestDeleteAccountHandler(new NetworkDataHandler::RequestDeleteAccount());
    static NetworkDataHandler::INetworkDataHandlerPtr sRequestListBlockedIPAddressesHandler(new NetworkDataHandler::RequestListBlockedIPAddresses());
    static NetworkDataHandler::INetworkDataHandlerPtr sRequestRemoveBlockedIPAddressHandler(new NetworkDataHandler::RequestRemoveBlockedIPAddress());
    static NetworkDataHandler::INetworkDataHandlerPtr sRequestAddBlockedIPAddressHandler(new NetworkDataHandler::RequestAddBlockedIPAddress());
    static NetworkDataHandler::INetworkDataHandlerPtr sRequestListBlockedAccountsHandler(new NetworkDataHandler::RequestListBlockedAccounts());
    static NetworkDataHandler::INetworkDataHandlerPtr sRequestRemoveBlockedAccountHandler(new NetworkDataHandler::RequestRemoveBlockedAccount());
    static NetworkDataHandler::INetworkDataHandlerPtr sRequestAddBlockedAccountHandler(new NetworkDataHandler::RequestAddBlockedAccount());


    return [](uint32_t tag)
    {
        switch(tag)
            {
            case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LOGIN:
                return sRequestLoginHandler;

            case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_ACCOUNT:
                return sRequestListAccountHandler;

            case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ADD_ACCOUNT:
                return sRequestAddAccountHandler;

            case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ENABLE_ACCOUNT:
                return sRequestEnableAccountHandler;

            case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_DISABLE_ACCOUNT:
                return sRequestDisableAccountHandler;

            case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_CHANGE_ACCOUNT_PASSWORD:
                return sRequestChangeAccountPasswordHandler;

            case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_DELETE_ACCOUNT:
                return sRequestDeleteAccountHandler;

            case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_BLOCKED_IP_ADDRESSES:
                return sRequestListBlockedIPAddressesHandler;

            case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_REMOVE_BLOCKED_IP_ADDRESS:
                return sRequestRemoveBlockedIPAddressHandler;

            case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ADD_BLOCKED_IP_ADDRESS:
                return sRequestAddBlockedIPAddressHandler;

            case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_BLOCKED_ACCOUNTS:
                return sRequestListBlockedAccountsHandler;

            case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_REMOVE_BLOCKED_ACCOUNT:
                return sRequestRemoveBlockedAccountHandler;

            case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ADD_BLOCKED_ACCOUNT:
                return sRequestAddBlockedAccountHandler;

            default:
                return NetworkDataHandler::INetworkDataHandlerPtr();
            }
    };
}

void ManagementServer::CleanUserConnection::handle(IConnectionPtr con, const unsigned char *data, size_t len)
{
    con->disconnect();
}
