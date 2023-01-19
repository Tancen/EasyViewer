#ifndef NETWORKDATAHANDLE_REQUESTENABLEACCOUNTHANDLER_H
#define NETWORKDATAHANDLE_REQUESTENABLEACCOUNTHANDLER_H


#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestEnableAccount : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}


#endif // NETWORKDATAHANDLE_REQUESTENABLEACCOUNTHANDLER_H
