#ifndef NETWORKDATAHANDLE_REQUESTVISITFILESHANDLER_H
#define NETWORKDATAHANDLE_REQUESTVISITFILESHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class RequestVisitFiles : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}

#endif // NETWORKDATAHANDLE_REQUESTVISITFILESHANDLER_H
