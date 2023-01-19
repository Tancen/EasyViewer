#ifndef NETWORKDATAHANDLE_REQUESTGOHOMEHANDLER_H
#define NETWORKDATAHANDLE_REQUESTGOHOMEHANDLER_H

#include "UsualRequestHandler.h"
#include "Global/Protocol/File/File.pb.h"

namespace  NetworkDataHandler
{
    class RequestGoHome : public UsualRequest
            <Global::Protocol::File::RequestGoHome,
            Global::Protocol::File::ResponseGoHome>
    {
    public:
        RequestGoHome();
    };
}

#endif // NETWORKDATAHANDLE_REQUESTGOHOMEHANDLER_H
