#ifndef NETWORKDATAHANDLE_RESPONSECREATETERMINAL2HANDLER_H
#define NETWORKDATAHANDLE_RESPONSECREATETERMINAL2HANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class ResponseCreateTerminal2 : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}

#endif // NETWORKDATAHANDLE_RESPONSECREATETERMINAL2HANDLER_H
