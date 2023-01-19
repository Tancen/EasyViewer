#ifndef NETWORKDATAHANDLE_REQUESTREMOVEBLOCKEDIPADDRESSHANDLER_H
#define NETWORKDATAHANDLE_REQUESTREMOVEBLOCKEDIPADDRESSHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestRemoveBlockedIPAddress : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}


#endif // NETWORKDATAHANDLE_REQUESTREMOVEBLOCKEDIPADDRESSHANDLER_H
