#ifndef NETWORKDATAHANDLE_RESPONSERENAMEENTRYHANDLER_H
#define NETWORKDATAHANDLE_RESPONSERENAMEENTRYHANDLER_H

#include "UsualResponseHandler.h"

namespace  NetworkDataHandler
{
    class ResponseRenameEntry : public UsualResponseHandler<
            Global::Protocol::File::ResponseRenameEntry>
    {
    public:
        ResponseRenameEntry();
    };
}


#endif // NETWORKDATAHANDLE_RESPONSERENAMEENTRYHANDLER_H
