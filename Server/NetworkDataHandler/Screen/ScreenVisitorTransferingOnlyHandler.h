#ifndef NETWORKDATAHANDLE_SCREENVISITORTRANSFERINGONLYHANDLER_H
#define NETWORKDATAHANDLE_SCREENVISITORTRANSFERINGONLYHANDLER_H

#include "../INetworkDataHandler.h"

namespace  NetworkDataHandler
{
    class ScreenVisitorTransferingOnly : public INetworkDataHandler
    {
    public:
        ScreenVisitorTransferingOnly(int tag);
        void handle(IConnectionPtr con, const unsigned char* data, size_t len);

    private:
        int m_tag;
    };
}

#endif // NETWORKDATAHANDLE_SCREENVISITORTRANSFERINGONLYHANDLER_H
