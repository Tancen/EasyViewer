#ifndef NETWORKDATAHANDLE_RESPONSELISTENTRIESHANDLER_H
#define NETWORKDATAHANDLE_RESPONSELISTENTRIESHANDLER_H

#include "UsualResponseHandler.h"

namespace  NetworkDataHandler
{
    class ResponseListEntries : public UsualResponseHandler
            <Global::Protocol::File::ResponseListEntries>
    {
    public:
        ResponseListEntries();
    };
}

#endif // NETWORKDATAHANDLE_RESPONSELISTENTRIESHANDLER_H
