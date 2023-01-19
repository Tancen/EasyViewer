#ifndef NETWORKDATAHANDLE_REQUESTOPENFILEHANDLER_H
#define NETWORKDATAHANDLE_REQUESTOPENFILEHANDLER_H


#include "UsualRequestHandler.h"
#include "Global/Protocol/File/File.pb.h"

namespace  NetworkDataHandler
{
    class RequestOpenFile : public UsualRequest
            <Global::Protocol::File::RequestOpenFile,
            Global::Protocol::File::ResponseOpenFile,
            FillSEQ, FillTaskId>
    {
    public:
        RequestOpenFile();
    };
}
#endif // NETWORKDATAHANDLE_REQUESTOPENFILEHANDLER_H
