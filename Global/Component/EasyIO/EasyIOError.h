#ifndef EASYIOERROR_H
#define EASYIOERROR_H

#include <string>
#include "EasyIODef.h"

namespace EasyIO
{
    namespace Error
    {
        constexpr const char* STR_FORCED_CLOSURE = "forced closure";

        std::string formatError(int err);
        int getSocketError(SOCKET socket);
    }
}

#endif // EASYIOERROR_H
