#ifndef NETWORKDATAHANDLE_REQUESTLISTACCOUNTHANDLER_H
#define NETWORKDATAHANDLE_REQUESTLISTACCOUNTHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestListAccount : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}

#endif // NETWORKDATAHANDLE_REQUESTLISTACCOUNTHANDLER_H
