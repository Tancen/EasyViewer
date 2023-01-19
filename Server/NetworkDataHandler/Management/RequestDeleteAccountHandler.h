#ifndef NETWORKDATAHANDLE_REQUESTDELETEACCOUNTHANDLER_H
#define NETWORKDATAHANDLE_REQUESTDELETEACCOUNTHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestDeleteAccount : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}


#endif // NETWORKDATAHANDLE_REQUESTDELETEACCOUNTHANDLER_H
