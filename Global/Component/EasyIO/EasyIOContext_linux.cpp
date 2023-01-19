#ifdef __linux__
#include "EasyIOContext_linux.h"

using namespace EasyIO::Context;

Context::Context(std::function<void (uint32_t)> callback)
    : m_callback(callback)
{
    this->events = 0;
    this->data.ptr = this;
}

Context::~Context()
{

}

void Context::setCallback(std::function<void (uint32_t)> callback)
{
    m_callback = callback;
}

void Context::update(uint32_t events)
{
    if (m_callback)
        m_callback(events);
}

#endif
