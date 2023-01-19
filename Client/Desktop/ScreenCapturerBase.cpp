#include "ScreenCapturerBase.h"
#include <assert.h>
#include <string.h>
#include <QDateTime>
#include "Global/Component/Logger/Logger.h"

ScreenCapturerBase::ScreenCapturerBase()
    :   m_forKeyFrame(0)
{

}

ScreenCapturerBase::~ScreenCapturerBase()
{
    m_encoder.reset();
}

bool ScreenCapturerBase::initEncoder(unsigned w, unsigned h)
{
    m_encoder.reset();

    ISVCEncoder* encoder;

    do
    {
        int err = WelsCreateSVCEncoder(&encoder);
        assert(!err && encoder);

        SEncParamBase param;
        memset (&param, 0, sizeof (param));

        param.iUsageType = SCREEN_CONTENT_REAL_TIME;
        param.iRCMode = RC_BITRATE_MODE;
        param.fMaxFrameRate = 60;
        param.iPicWidth = w;
        param.iPicHeight = h;
        param.iTargetBitrate = 300 * 1000 * 8;
        err = encoder->Initialize(&param);
        assert(!err);

        int foramt = videoFormatI420;
        err = encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &foramt);
        assert(!err);

        int skip = 1;
        err = encoder->SetOption(ENCODER_OPTION_RC_FRAME_SKIP, &skip);
        assert(!err);

        m_bufYUV.reset(new unsigned char[w * h * 4], [](unsigned char* p){ delete[] p; });
        m_w = w;
        m_h = h;

        m_encoder.reset(encoder);
        return true;
    } while(0);


    m_encoder.reset();
    return false;
}

bool ScreenCapturerBase::addFollower(void* key, ScreenFrameCallback screenFrameCallback)
{
    assert(screenFrameCallback);

    Callbacks callbacks;
    callbacks.screenFrameCallback = screenFrameCallback;

    m_lock.lock();
    bool b = m_followers.insert({key, callbacks}).second;
    m_lock.unlock();

    return b;
}

bool ScreenCapturerBase::removeFollower(void *key)
{
    std::string err;
    m_lock.lock();
    bool b = m_followers.erase(key);
    m_lock.unlock();

    return b;
}

void ScreenCapturerBase::start()
{
    m_stopped = false;
    froceKeyframe();
}

void ScreenCapturerBase::stop()
{
    m_stopped = true;
}

void ScreenCapturerBase::froceKeyframe()
{
    m_forKeyFrame += 1;
}

bool ScreenCapturerBase::pushScreenFrame(int frameCount, unsigned w, unsigned h, const unsigned char *data, size_t pitch)
{
    if (m_stopped)
        return true;

    if (m_w != w || m_h != h)
    {
        if (!initEncoder(w, h))
            return false;
    }

    SFrameBSInfo info;
    memset (&info, 0, sizeof (SFrameBSInfo));

    SSourcePicture pic;
    memset (&pic, 0, sizeof(SSourcePicture));
    pic.uiTimeStamp = 0;
    pic.iColorFormat = videoFormatI420;
    pic.iPicWidth = w;
    pic.iPicHeight = h;
    pic.pData[0] = m_bufYUV.get();
    pic.pData[1] = pic.pData[0] + w * h;
    pic.pData[2] = pic.pData[1] + w * h;
    pic.iStride[0] = pic.iStride[1] = pic.iStride[2] = w;

    fillPlanes(w, h, data, pitch, pic.pData[0], pic.pData[1], pic.pData[2], w);

    int forKeyFrame = --m_forKeyFrame;
    if (forKeyFrame >= 0)
    {
        m_encoder->ForceIntraFrame(true);
    }
    else if (forKeyFrame < 0)
        ++m_forKeyFrame;

    int err = m_encoder->EncodeFrame(&pic, &info);
    if (err != cmResultSuccess)
        return false;

    if (info.eFrameType != videoFrameTypeSkip)
    {
        for (int i = 0; i < info.iLayerNum; i++)
        {
            SLayerBSInfo* layer = &info.sLayerInfo[i];
            size_t size = 0;
            for (int j = 0; j < layer->iNalCount; j++)
                size += layer->pNalLengthInByte[j];

            publishScreenFrame(increaseSEQ(), w, h, layer->pBsBuf, size);
        }
    }
    return true;
}

void ScreenCapturerBase::publishScreenFrame(int64_t seq, unsigned w, unsigned h,  const unsigned char* data, size_t len)
{
    m_lock.lock();
    for (auto& it : m_followers)
        it.second.screenFrameCallback(seq, w, h, data, len);
    m_lock.unlock();
}

int64_t ScreenCapturerBase::increaseSEQ()
{
    int64_t ret = ++m_seq;
    return ret;
}

void ScreenCapturerBase::ISVCEncoderDeleter::operator()(ISVCEncoder *p)
{
    p->Uninitialize();
    WelsDestroySVCEncoder(p);
}
