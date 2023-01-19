#ifndef NETWORKDATAHANDLE_REQUESTCREATETERMINALHANDLER_H
#define NETWORKDATAHANDLE_REQUESTCREATETERMINALHANDLER_H


#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestCreateTerminal : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}


#endif // NETWORKDATAHANDLE_REQUESTCREATETERMINALHANDLER_H
