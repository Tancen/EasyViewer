#ifndef NETWORKDATAHANDLE_RESPONSEDELETEENTRYHANDLER_H
#define NETWORKDATAHANDLE_RESPONSEDELETEENTRYHANDLER_H

#include "UsualResponseHandler.h"

namespace  NetworkDataHandler
{
    class ResponseDeleteEntry : public UsualResponseHandler<Global::Protocol::File::ResponseDeleteEntry>
    {
    public:
        ResponseDeleteEntry();
    };
}

#endif // NETWORKDATAHANDLE_RESPONSEDELETEENTRYHANDLER_H
