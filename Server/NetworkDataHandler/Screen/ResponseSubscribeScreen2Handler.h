#ifndef NETWORKDATAHANDLE_RESPONSESUBSCRIBESCREEN2HANDLER_H
#define NETWORKDATAHANDLE_RESPONSESUBSCRIBESCREEN2HANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class ResponseSubscribeScreen2 : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}

#endif // NETWORKDATAHANDLE_RESPONSESUBSCRIBESCREEN2HANDLER_H
