#ifndef ISCREENCAPTURER_H
#define ISCREENCAPTURER_H

#include <functional>
#include <memory>
#include <list>

class IScreenCapturer
{
public:
    typedef  std::function<void(int64_t seq, unsigned w, unsigned h, const unsigned char* data, size_t len)>  ScreenFrameCallback;

public:
    virtual bool addFollower(void* key, ScreenFrameCallback screenFrameCallback) = 0;
    virtual bool removeFollower(void* key) = 0;
};

typedef std::shared_ptr<IScreenCapturer> IScreenCapturerPtr;

#endif // ISCREENCAPTURER_H
