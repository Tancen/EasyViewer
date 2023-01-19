#ifndef  NETWORKDATAHANDLE_REQUESTSUBSCRIBESCREENHANDLER_H
#define  NETWORKDATAHANDLE_REQUESTSUBSCRIBESCREENHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestSubscribeScreen : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}

#endif //  NETWORKDATAHANDLE_REQUESTSUBSCRIBESCREENHANDLER_H
