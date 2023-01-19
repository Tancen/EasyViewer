#include "RequestListBlockedIPAddressesHandler.h"
#include "UsualParseProtobufDataMacro.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Error.h"
#include "UserConnection.h"
#include "Global/Component/Logger/Logger.h"
#include "BlackList.h"
#include "Global/Protocol/Management/Server/Management.pb.h"

void NetworkDataHandler::RequestListBlockedIPAddresses::handle(IConnectionPtr con, const unsigned char *data, size_t len)
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

    USUAL_PARSE_PROTOBUF_DATA_MACRO(Global::Protocol::Server::Management::RequestListBlockedIPAddresses, request, data, len, usrcon->getSecondCryptology())

    Global::Protocol::Server::Management::ResponseListBlockedIPAddresses response;

    auto addresses = BlackList::share()->listIPAddresses();
    for (auto& it : addresses)
    {
        auto* addr = response.add_ip_addresses();
        addr->set_ip_address(it.ip);
        addr->set_until(it.until);
    }

    response.set_seq(request.seq());
    response.set_err_code(GLOBAL_PROTOCOL_ERR_NO_ERROR);

    EasyIO::ByteBuffer responseData;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LIST_BLOCKED_IP_ADDRESSES, response, responseData, usrcon->getSecondCryptology())
    usrcon->send(responseData);
}
