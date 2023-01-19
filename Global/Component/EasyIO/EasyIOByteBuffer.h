#ifndef EASYIOBYTEBUFFER_H
#define EASYIOBYTEBUFFER_H

#include <string>
#include <memory>

namespace EasyIO
{
    class ByteBuffer
    {
        struct PrivateData
        {
            std::shared_ptr<unsigned char> data;    // the buffer bytes stored
            size_t readerIndex = 0;
            size_t writerIndex = 0;
            size_t capacity = 0;
        };

    public:
        ByteBuffer();
        ByteBuffer(const ByteBuffer& o);
        ByteBuffer(ByteBuffer&& o);
        ByteBuffer(size_t capacity);
        ByteBuffer(const char *data, size_t size);
        ~ByteBuffer();

        ByteBuffer& operator= (const ByteBuffer& o);

        /**
        * @return A pointer to the bytes of this buffer
        */
        char* data();
        unsigned char* udata();

        /**
        * @return A pointer to the head of readable bytes
        */
        char* readableBytes();
        unsigned char* uReadableBytes();

        /**
        * @return The maximum number of bytes that can be stored in the byte buffer without forcing a reallocation
        */
        size_t capacity() const;

        /**
        * @return The current readerIndex in this buffer
        */
        size_t readerIndex() const;

        /**
        * @return The current writerIndex in this buffer
        */
        size_t writerIndex() const;

        /**
        * @return The number of readable bytes which is equal to (writerIndex - readerIndex)
        */
        size_t numReadableBytes() const;

        /**
        * @brief   Read this buffer's data to the specified destination. This method does modify readerIndex of this buffer
        * @param   dst the pointer to the destination
        * @param   len the number of bytes expected to be transferred
        * @return  The number of bytes transferred
        */
        size_t read(char* dst, size_t len);
        size_t read(unsigned char* dst, size_t len);

        /**
        * @brief   Read this buffer's data to the specified destination. This method does not modify readerIndex of this buffer
        * @param   dst the pointer to the destination
        * @param   len the number of bytes expected to be transferred
        * @return  The number of bytes transferred
        */
        size_t get(char* dst, size_t len);
        size_t get(unsigned char* dst, size_t len);

        /**
        * @brief   Write bytes to this buffer. This method does modify readerIndex of this buffer
        * @param   data the bytes to be transferred
        * @param   discardReadBytes if true, Reset data's readerIndex to zero
        */
        void write(ByteBuffer data, bool discardReadBytes = false);

        /**
        * @brief   Write bytes to this buffer. This method does modify writerIndex of this buffer
        * @param   data the pointer of bytes to be written
        * @param   len the number of bytes expected to be transferred
        */
        void write(const char* data, size_t len);
        void write(const unsigned char* data, size_t len);


        /**
        * @brief   Write the specified value to this buffer. This method does modify writerIndex of this buffer
        * @param   v the value to be written
        */
        void write(char v);
        void write(unsigned char v);
        void write(int16_t v);
        void write(uint16_t v);
        void write(int32_t v);
        void write(uint32_t v);
        void write(int64_t v);
        void write(uint64_t v);
        void write(float v);
        void write(double v);

        /**
        * @brief   Write the specified value to this buffer multiple times. This method does modify writerIndex of this buffer
        * @param   c the specified value
        * @param   len the number of expected times
        */
        void fill(char c, size_t len);

        /**
        * @brief Discard some unread bytes. This method does modify readerIndex or writerIndex of this buffer
        * @param len if len < 0, then discard all unread bytes. Otherwise, Discard the number of unread bytes of the specified length as far as possible
        */
        size_t discardReadBytes(int len = -1);

        /**
        * @brief   Move readerIndex in possible. Finally, the readerIndex between 0 and writerIndex
        * @param   offset the amount to be increased. If offset < 0, the readerIndex will be decreased
        */
        void moveReaderIndex(int offset);

        /**
        * @brief   Move writerIndex in possible.
        *          If writerIndex + offset < readerIndex, Both writerIndex and readerIndex are set to writerIndex + offset.
        *          If writerIndex + offset > capacity, writerIndex is set to capacity.
        *          Finally, the writerIndex between 0 and capacity.
        *          This method does modify readerIndex or writerIndex of this buffer.
        * @param   offset the amount to be increased. If offset < 0, the writerIndex will be decreased
        */
        void moveWriterIndex(int offset);

        /**
        * @brief   Set both readerIndex and writerIndex to zero.
        */
        void clear();

        /**
        * @brief   Clear all stored bytes and adjusts the capacity of this buffer.
        *          Set both readerIndex and writerIndex to zero.
        * @param   capacity the number of bytes this buffer can contain. If capacity <= 0, capacity is set to MIN_CAPACITY
        */
        void reset(size_t capacity = 0);

        /**
        * @brief   Expands the buffer capacity to make sure the number of writable bytes is equal to or greater than the specified value.
        * @param   bytes the expected minimum number of writable bytes
        */
        void ensureWritable(size_t bytes);

    private:
        size_t read(unsigned char* dst, size_t len, bool moveReaderIndex);

        static void free(unsigned char *p);

    private:
        std::shared_ptr<PrivateData> m_data;
    };
}

#endif
