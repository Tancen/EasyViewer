#ifndef NETWORKDATAHANDLE_REQUESTLISTENTRIESHANDLER_H
#define NETWORKDATAHANDLE_REQUESTLISTENTRIESHANDLER_H

#include "UsualRequestHandler.h"
#include "Global/Protocol/File/File.pb.h"

namespace  NetworkDataHandler
{
    class RequestListEntries : public UsualRequest
            <Global::Protocol::File::RequestListEntries,
            Global::Protocol::File::ResponseListEntries>
    {
    public:
        RequestListEntries();
    };
}

#endif // NETWORKDATAHANDLE_REQUESTLISTENTRIESHANDLER_H
