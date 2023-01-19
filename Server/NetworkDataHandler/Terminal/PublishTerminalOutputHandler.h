#ifndef NETWORKDATAHANDLE_PUBLISHTERMINALOUTPUTHANDLER_H
#define NETWORKDATAHANDLE_PUBLISHTERMINALOUTPUTHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class PublishTerminalOutput : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}
#endif // NETWORKDATAHANDLE_PUBLISHTERMINALOUTPUTHANDLER_H
