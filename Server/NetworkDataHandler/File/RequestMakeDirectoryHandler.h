#ifndef NETWORKDATAHANDLE_REQUESTMAKEDIRECTORYHANDLER_H
#define NETWORKDATAHANDLE_REQUESTMAKEDIRECTORYHANDLER_H

#include "UsualRequestHandler.h"
#include "Global/Protocol/File/File.pb.h"

namespace  NetworkDataHandler
{
    class RequestMakeDirectory : public UsualRequest
            <Global::Protocol::File::RequestMakeDirectory,
            Global::Protocol::File::ResponseMakeDirectory>
    {
    public:
        RequestMakeDirectory();
    };
}

#endif // NETWORKDATAHANDLE_REQUESTMAKEDIRECTORYHANDLER_H
