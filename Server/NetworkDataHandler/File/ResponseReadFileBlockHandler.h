#ifndef NETWORKDATAHANDLE_RESPONSEREADFILEBLOCKHANDLER_H
#define NETWORKDATAHANDLE_RESPONSEREADFILEBLOCKHANDLER_H

#include "UsualResponseHandler.h"

namespace  NetworkDataHandler
{
    class ResponseReadFileBlock : public UsualResponseHandler
            <Global::Protocol::File::ResponseReadFileBlock>
    {
    public:
        ResponseReadFileBlock();
    };
}


#endif // NETWORKDATAHANDLE_RESPONSEREADFILEBLOCKHANDLER_H
