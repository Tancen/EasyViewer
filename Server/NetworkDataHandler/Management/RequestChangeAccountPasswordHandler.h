#ifndef NETWORKDATAHANDLE_REQUESTCHANGEACCOUNTPASSWORDHANDLER_H
#define NETWORKDATAHANDLE_REQUESTCHANGEACCOUNTPASSWORDHANDLER_H


#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestChangeAccountPassword : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}

#endif // NETWORKDATAHANDLE_REQUESTCHANGEACCOUNTPASSWORDHANDLER_H
