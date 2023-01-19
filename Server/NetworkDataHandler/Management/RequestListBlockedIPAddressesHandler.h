#ifndef NETWORKDATAHANDLE_REQUESTLISTBLOCKEDIPADDRESSESHANDLER_H
#define NETWORKDATAHANDLE_REQUESTLISTBLOCKEDIPADDRESSESHANDLER_H


#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestListBlockedIPAddresses : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}


#endif // NETWORKDATAHANDLE_REQUESTLISTBLOCKEDIPADDRESSESHANDLER_H
