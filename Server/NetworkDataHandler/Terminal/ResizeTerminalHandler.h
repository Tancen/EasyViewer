#ifndef NETWORKDATAHANDLE_RESIZETERMINALHANDLER_H
#define NETWORKDATAHANDLE_RESIZETERMINALHANDLER_H


#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class ResizeTerminal : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}



#endif // NETWORKDATAHANDLE_RESIZETERMINALHANDLER_H
