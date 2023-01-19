#ifndef NETWORKDATAHANDLE_REQUESTLOGINHANDLER_H
#define NETWORKDATAHANDLE_REQUESTLOGINHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestLogin : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}

#endif // NETWORKDATAHANDLE_REQUESTLOGINHANDLER_H
