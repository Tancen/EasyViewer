#include "RequestEchoHandler.h"
#include "Global/Protocol/Common/Echo.pb.h"
#include "Global/Component/Logger/Logger.h"
#include "UserConnection.h"
#include "Global/Protocol/Protocol.h"
#include "UsualParseProtobufDataMacro.h"

void NetworkDataHandler::RequestEcho::handle(IConnectionPtr con, const unsigned char *data, size_t len)
{
    UserConnection* usrcon = dynamic_cast<UserConnection*>(con.get());
    if (!usrcon)
    {
        Logger::warning("%s:%d - !usrcon, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    const Account *usrInfo = usrcon->getBindingUser();
    if (!usrInfo)
    {
        Logger::warning("%s:%d - !usrInfo, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        con->peerIP().c_str(), con->peerPort());
        con->disconnect();
        return;
    }

    USUAL_PARSE_PROTOBUF_DATA_MACRO(Global::Protocol::Common::RequestEcho, request, data, len, usrcon->getSecondCryptology())

    {
        Global::Protocol::Common::ResponseEcho response;
        response.set_seq(request.seq());
        response.set_time(request.time());
        EasyIO::ByteBuffer data;
        GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_ECHO, response, data, usrcon->getSecondCryptology())
        usrcon->send(data);
    }

}
