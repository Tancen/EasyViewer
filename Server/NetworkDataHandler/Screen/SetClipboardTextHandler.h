#ifndef NETWORKDATAHANDLE_SETCLIPBOARDTEXTHANDLER_H
#define NETWORKDATAHANDLE_SETCLIPBOARDTEXTHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class SetClipboardText : public INetworkDataHandler
    {
    public:
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);
    };
}

#endif // NETWORKDATAHANDLE_SETCLIPBOARDTEXTHANDLER_H
