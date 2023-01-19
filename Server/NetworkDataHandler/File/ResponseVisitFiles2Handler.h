#ifndef NETWORKDATAHANDLE_RESPONSEVISITFILES2HANDLER_H
#define NETWORKDATAHANDLE_RESPONSEVISITFILES2HANDLER_H


#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class ResponseVisitFiles2 : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}

#endif // NETWORKDATAHANDLE_RESPONSEVISITFILES2HANDLER_H
