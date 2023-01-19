#ifndef NETWORKDATAHANDLE_PUBLISHCURSORPOSITIONHANDLER_H
#define NETWORKDATAHANDLE_PUBLISHCURSORPOSITIONHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class PublishCursorPosition : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}

#endif // NETWORKDATAHANDLE_PUBLISHCURSORPOSITIONHANDLER_H
