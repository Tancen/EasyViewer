#if  defined(WIN32) || defined(WIN64)
#ifndef EASYIOCONTEXT_H
#define EASYIOCONTEXT_H

#include <Winsock2.h>
#include <memory>
#include <functional>
#include "EasyIOByteBuffer.h"
#include <atomic>

namespace EasyIO
{
    class Context : public OVERLAPPED
    {
    public:
        enum Flag{ INBOUND, OUTBOUND};

    public:
        Context(Flag flag);
        Context(ByteBuffer buffer, Flag flag);

        Flag flag();
        void increase();
        void decrease();

        void increaseProgress(size_t increase);
        void error(int err);

        ByteBuffer buffer();
        WSABUF* WSABuf();
        bool finished();

    public:
        std::function<void(Context*, size_t increase)> onDone;
        std::function<void(Context*, int err)> onError;

    protected:
        ~Context();

    private:
        Flag m_flag;
        ByteBuffer m_buffer;
        std::atomic<int> m_ref;
        WSABUF m_wsaBuffer;
    };
}

#endif
#endif
