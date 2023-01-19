#ifndef NETWORKDATAHANDLE_KICKOUTTERMINALVISITORHANDLER_H
#define NETWORKDATAHANDLE_KICKOUTTERMINALVISITORHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class KickoutTerminalVisitor : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}

#endif // NETWORKDATAHANDLE_KICKOUTTERMINALVISITORHANDLER_H
