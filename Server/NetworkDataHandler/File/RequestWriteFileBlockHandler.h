#ifndef NETWORKDATAHANDLE_REQUESTWRITEFILEBLOCKHANDLER_H
#define NETWORKDATAHANDLE_REQUESTWRITEFILEBLOCKHANDLER_H

#include "UsualRequestHandler.h"
#include "Global/Protocol/File/File.pb.h"

namespace  NetworkDataHandler
{
    class RequestWriteFileBlock : public UsualRequest
            <Global::Protocol::File::RequestWriteFileBlock,
            Global::Protocol::File::ResponseWriteFileBlock,
            NoFillSEQ, FillTaskId>
    {
    public:
        RequestWriteFileBlock();
    };
}


#endif // NETWORKDATAHANDLE_REQUESTWRITEFILEBLOCKHANDLER_H
