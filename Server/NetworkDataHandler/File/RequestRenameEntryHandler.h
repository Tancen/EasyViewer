#ifndef NETWORKDATAHANDLE_REQUESTRENAMEENTRYHANDLER_H
#define NETWORKDATAHANDLE_REQUESTRENAMEENTRYHANDLER_H

#include "UsualRequestHandler.h"
#include "Global/Protocol/File/File.pb.h"

namespace  NetworkDataHandler
{
    class RequestRenameEntry : public UsualRequest
            <Global::Protocol::File::RequestRenameEntry,
            Global::Protocol::File::ResponseRenameEntry>
    {
    public:
        RequestRenameEntry();
    };
}

#endif // NETWORKDATAHANDLE_REQUESTRENAMEENTRYHANDLER_H
