#ifndef NETWORKDATAHANDLE_REQUESTGOUPHANDLER_H
#define NETWORKDATAHANDLE_REQUESTGOUPHANDLER_H

#include "UsualRequestHandler.h"
#include "Global/Protocol/File/File.pb.h"

namespace  NetworkDataHandler
{
    class RequestGoUp : public UsualRequest
            <Global::Protocol::File::RequestGoUp,
            Global::Protocol::File::ResponseGoUp>
    {
    public:
        RequestGoUp();
    };
}


#endif // NETWORKDATAHANDLE_REQUESTGOUPHANDLER_H
