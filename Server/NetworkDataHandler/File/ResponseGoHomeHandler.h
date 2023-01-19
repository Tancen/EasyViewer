#ifndef NETWORKDATAHANDLE_RESPONSEGOHOMEHANDLER_H
#define NETWORKDATAHANDLE_RESPONSEGOHOMEHANDLER_H

#include "UsualResponseHandler.h"

namespace  NetworkDataHandler
{
    class ResponseGoHome : public UsualResponseHandler
            <Global::Protocol::File::ResponseGoHome>
    {
    public:
        ResponseGoHome();
    };
}

#endif // NETWORKDATAHANDLE_RESPONSEGOHOMEHANDLER_H
