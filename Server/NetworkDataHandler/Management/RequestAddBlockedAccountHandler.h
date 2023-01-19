#ifndef NETWORKDATAHANDLE_REQUESTADDBLOCKEDACCOUNTHANDLER_H
#define NETWORKDATAHANDLE_REQUESTADDBLOCKEDACCOUNTHANDLER_H


#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestAddBlockedAccount : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}


#endif // NETWORKDATAHANDLE_REQUESTADDBLOCKEDACCOUNTHANDLER_H
