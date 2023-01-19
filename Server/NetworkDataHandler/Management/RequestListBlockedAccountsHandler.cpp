#include "RequestListBlockedAccountsHandler.h"
#include "UsualParseProtobufDataMacro.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Error.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "BlackList.h"
#include "Global/Protocol/Management/Server/Management.pb.h"

void NetworkDataHandler::RequestListBlockedAccounts::handle(IConnectionPtr con, const unsigned char *data, size_t len)
{
    UserConnection* usrcon = dynamic_cast<UserConnection*>(con.get());
    if (!usrcon)
    {
        Logger::warning("%s:%d - !usrcon, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    if (usrcon->getRole() != GLOBAL_CONNECTION_ROLE_FLAG_MANAGER)
    {
        Logger::warning("%s:%d - check role failed, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    USUAL_PARSE_PROTOBUF_DATA_MACRO(Global::Protocol::Server::Management::RequestListBlockedAccounts, request, data, len, usrcon->getSecondCryptology())

    Global::Protocol::Server::Management::ResponseListBlockedAccounts response;

    auto accounts = BlackList::share()->listAccounts();
    for (auto& it : accounts)
    {
        auto account = response.add_accounts();
        account->set_user_id(it.userId);
        account->set_account(it.account);
        account->set_until(it.until);
    }

    response.set_seq(request.seq());
    response.set_err_code(GLOBAL_PROTOCOL_ERR_NO_ERROR);

    EasyIO::ByteBuffer responseData;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LIST_BLOCKED_ACCOUNTS, response, responseData, usrcon->getSecondCryptology())
    usrcon->send(responseData);
}
