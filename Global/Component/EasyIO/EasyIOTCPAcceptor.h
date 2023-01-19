#ifndef EASYIOTCPACCEPTOR_H
#define EASYIOTCPACCEPTOR_H

#include <functional>
#include <thread>
#include "EasyIODef.h"

namespace EasyIO
{
    namespace TCP
    {
        class Acceptor
        {
        public:
            Acceptor();
            ~Acceptor();

            bool accept(unsigned short port, int backlog, int& err);

        private:
            void execute();

        public:
            std::function<void (SOCKET sock)> onAccepted;

        private:
            SOCKET m_socket;
            bool m_exit;
            std::thread m_thread;
        };
    }
}

#endif
