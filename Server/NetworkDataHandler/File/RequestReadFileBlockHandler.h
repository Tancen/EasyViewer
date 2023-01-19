#ifndef NETWORKDATAHANDLE_REQUESTREADFILEBLOCKHANDLER_H
#define NETWORKDATAHANDLE_REQUESTREADFILEBLOCKHANDLER_H

#include "UsualRequestHandler.h"
#include "Global/Protocol/File/File.pb.h"

namespace  NetworkDataHandler
{
    class RequestReadFileBlock : public UsualRequest
            <Global::Protocol::File::RequestReadFileBlock,
            Global::Protocol::File::ResponseReadFileBlock,
            NoFillSEQ, FillTaskId>
    {
    public:
        RequestReadFileBlock();
    };
}


#endif // NETWORKDATAHANDLE_REQUESTREADFILEBLOCKHANDLER_H
