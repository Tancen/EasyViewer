#ifndef EASYIOUDPICLIENT_H
#define EASYIOUDPICLIENT_H

#include "EasyIOByteBuffer.h"
#include "EasyIOUDPISocket.h"
#include "EasyIOUDPIServer.h"
#include <functional>

namespace EasyIO
{
    namespace UDP
    {
        class IClient : virtual public IServer
        {
        public:
            IClient(){}
            virtual ~IClient(){}
            bool open(){ return open(0); };

        protected:
            using IServer::open;
        };

        typedef std::shared_ptr<IClient> IClientPtr;
    }
}

#endif
