#ifndef NETWORKDATAHANDLE_REQUESTDELETEENTRYHANDLER_H
#define NETWORKDATAHANDLE_REQUESTDELETEENTRYHANDLER_H

#include "UsualRequestHandler.h"
#include "Global/Protocol/File/File.pb.h"

namespace  NetworkDataHandler
{
    class RequestDeleteEntry : public UsualRequest
            <Global::Protocol::File::RequestDeleteEntry,
            Global::Protocol::File::ResponseDeleteEntry>
    {
    public:
        RequestDeleteEntry();
    };
}

#endif // NETWORKDATAHANDLE_REQUESTDELETEENTRYHANDLER_H
