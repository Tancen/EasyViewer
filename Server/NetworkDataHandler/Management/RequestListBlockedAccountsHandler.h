#ifndef NETWORKDATAHANDLE_REQUESTLISTBLOCKEDACCOUNTSHANDLER_H
#define NETWORKDATAHANDLE_REQUESTLISTBLOCKEDACCOUNTSHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestListBlockedAccounts : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}


#endif // NETWORKDATAHANDLE_REQUESTLISTBLOCKEDACCOUNTSHANDLER_H
