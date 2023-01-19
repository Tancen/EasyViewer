#ifndef NETWORKDATAHANDLE_REQUESTDISABLEACCOUNT_H
#define NETWORKDATAHANDLE_REQUESTDISABLEACCOUNT_H


#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestDisableAccount : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}


#endif // NETWORKDATAHANDLE_REQUESTDISABLEACCOUNT_H
