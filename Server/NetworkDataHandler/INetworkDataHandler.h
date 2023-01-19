#ifndef NETWORKDATAHANDLE_INETWORKDATAHANDLER_H
#define NETWORKDATAHANDLE_INETWORKDATAHANDLER_H

#include "IConnection.h"

namespace  NetworkDataHandler
{
    class INetworkDataHandler
    {
    public:
        virtual void handle(IConnectionPtr con, const unsigned char* data, size_t len) = 0;
    };

    typedef std::shared_ptr<INetworkDataHandler> INetworkDataHandlerPtr;
}

#endif // NETWORKDATAHANDLE_INETWORKDATAHANDLER_H
