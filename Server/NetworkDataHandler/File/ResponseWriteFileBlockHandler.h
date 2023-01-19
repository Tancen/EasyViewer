#ifndef NETWORKDATAHANDLE_RESPONSEWRITEFILEBLOCKHANDLER_H
#define NETWORKDATAHANDLE_RESPONSEWRITEFILEBLOCKHANDLER_H

#include "UsualResponseHandler.h"

namespace  NetworkDataHandler
{
    class ResponseWriteFileBlock : public UsualResponseHandler
            <Global::Protocol::File::ResponseWriteFileBlock>
    {
    public:
        ResponseWriteFileBlock();
    };
}


#endif // NETWORKDATAHANDLE_RESPONSEWRITEFILEBLOCKHANDLER_H
