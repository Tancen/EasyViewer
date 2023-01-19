#ifndef NETWORKDATAHANDLE_REQUESTADDBLOCKEDIPADDRESSHANDLER_H
#define NETWORKDATAHANDLE_REQUESTADDBLOCKEDIPADDRESSHANDLER_H


#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestAddBlockedIPAddress : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}


#endif // NETWORKDATAHANDLE_REQUESTADDBLOCKEDIPADDRESSHANDLER_H
