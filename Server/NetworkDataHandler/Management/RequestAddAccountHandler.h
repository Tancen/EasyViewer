#ifndef NETWORKDATAHANDLE_REQUESTADDACCOUNTHANDLER_H
#define NETWORKDATAHANDLE_REQUESTADDACCOUNTHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestAddAccount : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}

#endif // NETWORKDATAHANDLE_REQUESTADDACCOUNTHANDLER_H
