#include "EasyIOByteBuffer.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

using namespace EasyIO;

#define MIN_CAPACITY 4096

ByteBuffer::ByteBuffer()
    : ByteBuffer(MIN_CAPACITY)
{

}

ByteBuffer::ByteBuffer(const ByteBuffer &o)
    :   m_data(o.m_data)
{

}

ByteBuffer::ByteBuffer(ByteBuffer &&o)
    :   m_data(std::move(o.m_data))
{

}

ByteBuffer::ByteBuffer(size_t capacity)
    :   m_data(new PrivateData())
{
    reset(capacity);
}

ByteBuffer::ByteBuffer(const char *data, size_t size)
    : ByteBuffer()
{
    write(data, size);
}

ByteBuffer::~ByteBuffer()
{

}

ByteBuffer &ByteBuffer::operator=(const ByteBuffer &o)
{
    this->m_data = o.m_data;
    return *this;
}

char *ByteBuffer::data()
{
    return (char*)udata();
}

unsigned char *ByteBuffer::udata()
{
    return m_data->data.get();
}

char *ByteBuffer::readableBytes()
{
    return (char*)uReadableBytes();
}

unsigned char *ByteBuffer::uReadableBytes()
{
    return m_data->data.get() + m_data->readerIndex;
}

size_t ByteBuffer::capacity() const
{
    return m_data->capacity;
}

size_t ByteBuffer::readerIndex() const
{
    return m_data->readerIndex;
}

size_t ByteBuffer::writerIndex() const
{
    return m_data->writerIndex;
}

size_t ByteBuffer::numReadableBytes() const
{
    assert(m_data->writerIndex >= m_data->readerIndex);
    return m_data->writerIndex - m_data->readerIndex;
}

size_t ByteBuffer::read(unsigned char *dst, size_t len, bool moveReaderIndex)
{
    size_t ret = std::min(len, numReadableBytes());
    if (!ret)
        return 0;
    memcpy(dst, m_data->data.get() + m_data->readerIndex, ret);
    if (moveReaderIndex)
    {
        m_data->readerIndex += ret;
        assert(m_data->readerIndex <= m_data->writerIndex);
        if (m_data->readerIndex == m_data->writerIndex)
        {
            m_data->readerIndex = 0;
            m_data->writerIndex = 0;
        }
    }

    return ret;
}

size_t ByteBuffer::read(char *dst, size_t len)
{
    return read((unsigned char *)dst, len, true);
}

size_t ByteBuffer::read(unsigned char *dst, size_t len)
{
    return read(dst, len, true);
}

size_t ByteBuffer::get(char *dst, size_t len)
{
    return read((unsigned char*)dst, len, false);
}

size_t ByteBuffer::get(unsigned char *dst, size_t len)
{
    return read(dst, len, false);
}

void ByteBuffer::write(ByteBuffer data, bool discardReadBytes)
{
    size_t l = data.numReadableBytes();
    if (!l)
        return;

    write(data.readableBytes(), l);
    if (discardReadBytes)
        data.discardReadBytes(l);
}

void ByteBuffer::write(const char *data, size_t len)
{
    write((const unsigned char*)data, len);
}

void ByteBuffer::write(const unsigned char *data, size_t len)
{
    if (!len)
        return;
    ensureWritable(len);
    memcpy(m_data->data.get() + m_data->writerIndex, data, len);
    m_data->writerIndex += len;
}

void ByteBuffer::write(char v)
{
    write(&v, sizeof(v));
}

void ByteBuffer::write(unsigned char v)
{
    write((char*)&v, sizeof(v));
}

void ByteBuffer::write(int16_t v)
{
    write((char*)&v, sizeof(v));
}

void ByteBuffer::write(uint16_t v)
{
    write((char*)&v, sizeof(v));
}

void ByteBuffer::write(int32_t v)
{
    write((char*)&v, sizeof(v));
}

void ByteBuffer::write(uint32_t v)
{
    write((char*)&v, sizeof(v));
}

void ByteBuffer::write(int64_t v)
{
    write((char*)&v, sizeof(v));
}

void ByteBuffer::write(uint64_t v)
{
    write((char*)&v, sizeof(v));
}

void ByteBuffer::write(float v)
{
    write((char*)&v, sizeof(v));
}

void ByteBuffer::write(double v)
{
    write((char*)&v, sizeof(v));
}

void ByteBuffer::fill(char c, size_t len)
{
    if (!len)
        return;
    ensureWritable(len);
    memset(m_data->data.get() + m_data->writerIndex, c, len);
    m_data->writerIndex += len;
}

void ByteBuffer::clear()
{
    m_data->readerIndex = m_data->writerIndex = 0;
}

void ByteBuffer::reset(size_t capacity)
{
    if (capacity < MIN_CAPACITY)
        capacity = MIN_CAPACITY;
    else
    {
        auto adjustSize = [](size_t v)
        {
            size_t n = v;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            n |= n >> 32;
            n += 1;
            return (n >> 1) == v ? v : n;
        };
        capacity = adjustSize(capacity);
    }

    m_data->data.reset(new unsigned char[capacity], free);
    m_data->capacity = capacity;
    m_data->readerIndex = 0;
    m_data->writerIndex = 0;
}

size_t ByteBuffer::discardReadBytes(int len)
{
    size_t l0;
    if (len < 0)
        l0 = numReadableBytes();
    else
        l0 = len;

    size_t l = std::min(l0, numReadableBytes());
    m_data->readerIndex += l;
    assert(m_data->readerIndex <= m_data->writerIndex);
    if (m_data->readerIndex == m_data->writerIndex)
    {
        m_data->readerIndex = 0;
        m_data->writerIndex = 0;
    }

    return l;
}

void ByteBuffer::moveReaderIndex(int offset)
{
    long long index = m_data->readerIndex + offset;
    if (index < 0)
        m_data->readerIndex = 0;
    else if (index >= m_data->writerIndex)
        m_data->readerIndex = m_data->writerIndex;
    else
        m_data->readerIndex = index;

}

void ByteBuffer::moveWriterIndex(int offset)
{
    long long index = m_data->writerIndex + offset;
    if (index < 0)
    {
        m_data->readerIndex = 0;
        m_data->writerIndex = 0;
    }
    else if (index < (long long)m_data->readerIndex)
    {
        m_data->readerIndex = m_data->writerIndex = index;
    }
    else if (index >= m_data->capacity)
        m_data->writerIndex = m_data->capacity;
    else
        m_data->writerIndex = index;
}


void ByteBuffer::ensureWritable(size_t len)
{
    size_t r = m_data->capacity - m_data->writerIndex;
    if (r < len)
    {
        size_t l = numReadableBytes();
        if (m_data->readerIndex >= len && m_data->readerIndex > (m_data->capacity >> 1))
        {
            memmove(m_data->data.get(), m_data->data.get() + m_data->readerIndex, l);
        }
        else
        {
            size_t c = m_data->capacity << 1;
            while (true)
            {
                if (c >= m_data->writerIndex + len)
                    break;
                c <<= 1;
            }
            std::shared_ptr<unsigned char> old = m_data->data;
            m_data->data.reset(new unsigned char[c], free);
            memcpy(m_data->data.get(), old.get() + m_data->readerIndex, l);
            m_data->capacity = c;
        }
        m_data->readerIndex = 0;
        m_data->writerIndex = l;
    }
}

void ByteBuffer::free(unsigned char *p)
{
	delete[] p;
}

