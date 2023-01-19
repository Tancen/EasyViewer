#ifndef NETWORKDATAHANDLE_REQUESTREMOVEBLOCKEDACCOUNTHANDLER_H
#define NETWORKDATAHANDLE_REQUESTREMOVEBLOCKEDACCOUNTHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestRemoveBlockedAccount : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}


#endif // NETWORKDATAHANDLE_REQUESTREMOVEBLOCKEDACCOUNTHANDLER_H
