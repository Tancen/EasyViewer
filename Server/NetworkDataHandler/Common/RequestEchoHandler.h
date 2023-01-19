#ifndef NETWORKDATAHANDLE_REQUESTECHOHANDLER_H
#define NETWORKDATAHANDLE_REQUESTECHOHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestEcho : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}

#endif // NETWORKDATAHANDLE_REQUESTECHOHANDLER_H
