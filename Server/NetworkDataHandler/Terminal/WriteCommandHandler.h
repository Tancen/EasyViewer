#ifndef NETWORKDATAHANDLE_WRITECOMMANDHANDLER_H
#define NETWORKDATAHANDLE_WRITECOMMANDHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class WriteCommand : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}


#endif // NETWORKDATAHANDLE_WRITECOMMANDHANDLER_H
