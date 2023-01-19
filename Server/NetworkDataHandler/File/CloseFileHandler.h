#ifndef NETWORKDATAHANDLE_CLOSEFILEHANDLER_H
#define NETWORKDATAHANDLE_CLOSEFILEHANDLER_H

#include "UsualResponseHandler.h"

namespace  NetworkDataHandler
{
    class CloseFile : public UsualResponseHandler
            <Global::Protocol::File::CloseFile>
    {
    public:
        CloseFile();
    };
}


#endif // NETWORKDATAHANDLE_CLOSEFILEHANDLER_H
