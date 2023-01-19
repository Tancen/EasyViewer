#ifndef SCREENCAPTURERBASE_H
#define SCREENCAPTURERBASE_H

#include "IScreenCapturer.h"
#include <mutex>
#include <map>
#include <memory>
#include <atomic>
#include "wels/codec_api.h"

class ScreenCapturerBase : public IScreenCapturer
{
    struct Callbacks
    {
        ScreenFrameCallback screenFrameCallback;
    };

    class ISVCEncoderDeleter
    {
    public:
        void operator() (ISVCEncoder* p);
    };

public:
    ScreenCapturerBase();
    virtual ~ScreenCapturerBase();

    bool initEncoder(unsigned w, unsigned h);

    bool addFollower(void* key, ScreenFrameCallback screenFrameCallback);
    bool removeFollower(void* key);

    void start();
    void stop();
    void froceKeyframe();

protected:
    int64_t seq();

    bool pushScreenFrame(int frameCount, unsigned w, unsigned h, const unsigned char* data, size_t pitch);
    void publishScreenFrame(int64_t seq, unsigned w, unsigned h, const unsigned char* data, size_t len);

    virtual void fillPlanes(unsigned w, unsigned h, const unsigned char* src, size_t pitch,
                            unsigned char* dstY, unsigned char* dstU, unsigned char* dstV, size_t dstStride) = 0;

private:
    inline int64_t increaseSEQ();

protected:
    std::atomic<int64_t> m_seq = 0;
    std::unique_ptr<ISVCEncoder, ISVCEncoderDeleter> m_encoder;
    unsigned m_w;
    unsigned m_h;
    std::atomic<int> m_forKeyFrame;
    std::shared_ptr<unsigned char> m_bufYUV;
    std::map<void*, Callbacks> m_followers;
    bool m_stopped = true;
    std::recursive_mutex m_lock;
};

#endif // SCREENCAPTURERBASE_H
