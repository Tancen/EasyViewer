#ifndef NETWORKDATAHANDLE_RESPONSEGOUPHANDLER_H
#define NETWORKDATAHANDLE_RESPONSEGOUPHANDLER_H

#include "UsualResponseHandler.h"

namespace  NetworkDataHandler
{
    class ResponseGoUp : public UsualResponseHandler
            <Global::Protocol::File::ResponseGoUp>
    {
    public:
        ResponseGoUp();
    };
}

#endif // NETWORKDATAHANDLE_RESPONSEGOUPHANDLER_H
