#ifndef NETWORKDATAHANDLE_FACTORY_H
#define NETWORKDATAHANDLE_FACTORY_H

#include "INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class Factory
    {
    public:
        static INetworkDataHandlerPtr create(int tag);
    };
}

#endif // NETWORKDATAHANDLE_FACTORY_H
