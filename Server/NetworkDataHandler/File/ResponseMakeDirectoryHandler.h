#ifndef NETWORKDATAHANDLE_RESPONSEMAKEDIRECTORYHANDLER_H
#define NETWORKDATAHANDLE_RESPONSEMAKEDIRECTORYHANDLER_H

#include "UsualResponseHandler.h"

namespace  NetworkDataHandler
{
    class ResponseMakeDirectory : public UsualResponseHandler<Global::Protocol::File::ResponseMakeDirectory>
    {
    public:
        ResponseMakeDirectory();
    };
}

#endif // NETWORKDATAHANDLE_RESPONSEMAKEDIRECTORYHANDLER_H
