#ifndef NETWORKDATAHANDLE_PUBLISHSCREENFRAMEHANDLER_H
#define NETWORKDATAHANDLE_PUBLISHSCREENFRAMEHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class PublishScreenFrame : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}


#endif // NETWORKDATAHANDLE_PUBLISHSCREENFRAMEHANDLER_H
