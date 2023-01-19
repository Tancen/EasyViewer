#ifndef EASYIOIEVENTLOOP_H
#define EASYIOIEVENTLOOP_H

#include <memory>

namespace EasyIO
{
    class IEventLoop;
    typedef std::shared_ptr<IEventLoop> IEventLoopPtr;

    class IEventLoop : public std::enable_shared_from_this<IEventLoop>
    {
    public:
        IEventLoop(){}
        virtual ~IEventLoop(){}

        virtual IEventLoopPtr share() = 0;
    };
}

#endif
