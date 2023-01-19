#ifndef NETWORKDATAHANDLE_RESPONSEOPENFILEHANDLER_H
#define NETWORKDATAHANDLE_RESPONSEOPENFILEHANDLER_H

#include "UsualResponseHandler.h"

namespace  NetworkDataHandler
{
    class ResponseOpenFileTask : public UsualResponseHandler
            <Global::Protocol::File::ResponseOpenFile>
    {
    public:
        ResponseOpenFileTask();
    };
}

#endif // NETWORKDATAHANDLE_RESPONSEOPENFILEHANDLER_H
