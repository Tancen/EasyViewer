#ifdef __linux__
#ifndef EASYIOCONTEXT_H
#define EASYIOCONTEXT_H

#include <memory>
#include <functional>
#include <sys/epoll.h>

namespace EasyIO
{
    namespace Context
    {

        class Context : public epoll_event
        {
        public:
            Context(std::function<void(uint32_t)> callback);
            ~Context();

            void setCallback(std::function<void(uint32_t)> callback);
            void update(uint32_t events);

        private:
            std::function<void(uint32_t)> m_callback;
        };
        typedef std::shared_ptr<Context> ContextPtr;
    }
}

#endif

#endif
