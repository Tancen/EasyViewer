#include <QtGlobal>

#ifdef Q_OS_WIN
#include "ScreenCapturer_win.h"
#include <Windows.h>
#include "Global/Component/Logger/Logger.h"
#include "Global/Define.h"
#include <QCursor>
#include <dxgi1_3.h>
#include <d3d9.h>
#include <comdef.h>
#include <QString>
#include <QDateTime>
#include "libyuv.h"

#define SAFE_RELEASE(p) \
    if (p) \
    { \
        p->Release(); \
        p = nullptr; \
    }

ScreenCapturer* ScreenCapturer::s_this = nullptr;

ScreenCapturer *ScreenCapturer::share()
{
    return s_this;
}

bool ScreenCapturer::init()
{
    assert(s_this == nullptr);
    s_this = new ScreenCapturer();
    do
    {
        HRESULT err;
        const D3D_FEATURE_LEVEL FEATURE_LEVELS[] =
        {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_1
        };

        D3D_FEATURE_LEVEL level;
        err = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                    D3D11_CREATE_DEVICE_VIDEO_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
                    FEATURE_LEVELS,
                    sizeof(FEATURE_LEVELS) / sizeof(D3D_FEATURE_LEVEL),
                    D3D11_SDK_VERSION, &s_this->m_d3dDevice, &level, &s_this->m_d3dContext);
        if (FAILED(err))
        {
            Logger::error("%s:%d - D3D11CreateDevice failed[%x]", __PRETTY_FUNCTION__, __LINE__, err);
            break;
        }
        assert(s_this->m_d3dDevice && s_this->m_d3dContext);

        err = s_this->m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&s_this->m_dxgiDevice);
        if (FAILED(err))
        {
            Logger::error("%s:%d - d3dDevice->QueryInterface failed[%x]", __PRETTY_FUNCTION__, __LINE__, err);
            break;
        }
        assert(s_this->m_dxgiDevice);


        IDXGIFactory* factory = nullptr;
        err = CreateDXGIFactory(__uuidof(IDXGIFactory) ,(void**)&factory);

        IDXGIAdapter *adapter = nullptr;
        DXGI_ADAPTER_DESC desc;
        for (int i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            adapter->GetDesc(&desc);
        }

        err = s_this->m_dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&s_this->m_dxgiAdapter);
        if (FAILED(err))
        {
            Logger::error("%s:%d - dxgiDevice->GetParent failed[%x]", __PRETTY_FUNCTION__, __LINE__, err);
            break;
        }
        assert(s_this->m_dxgiAdapter);

        bool success = false;
        int index = 0;
        do
        {
            SAFE_RELEASE(s_this->m_dxgiOutput)
            SAFE_RELEASE(s_this->m_dxgiOutput1)
            SAFE_RELEASE(s_this->m_d3dVideoDevice)
            SAFE_RELEASE(s_this->m_d3dVideoContext)

            DXGI_ADAPTER_DESC desc;
            s_this->m_dxgiAdapter->GetDesc(&desc);
            err = s_this->m_dxgiAdapter->EnumOutputs(index++, &s_this->m_dxgiOutput);
            if (err == DXGI_ERROR_NOT_FOUND)
            {
                Logger::error("not found suitable adapter");
                break;
            }

            if (FAILED(err))
            {
                Logger::error("%s:%d - dxgiAdapter->EnumOutputs failed[%x]", __PRETTY_FUNCTION__, __LINE__, err);
                continue;
            }
            assert(s_this->m_dxgiOutput);

            err = s_this->m_dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&s_this->m_dxgiOutput1);
            if (FAILED(err))
            {
                Logger::error("%s:%d - dxgiOutput->QueryInterface failed[%x]", __PRETTY_FUNCTION__, __LINE__, err);
                continue;
            }
            assert(s_this->m_dxgiOutput1);

            err = s_this->m_d3dDevice->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&s_this->m_d3dVideoDevice);
            if (FAILED(err))
            {
                Logger::error("%s:%d - d3dDevice->QueryInterface failed[%x]", __PRETTY_FUNCTION__, __LINE__, err);
                continue;
            }
            assert(s_this->m_d3dVideoDevice);

            err = s_this->m_d3dContext->QueryInterface(__uuidof(ID3D11VideoContext), (void**)&s_this->m_d3dVideoContext);
            if (FAILED(err))
            {
                Logger::error("%s:%d - m_d3dContext->QueryInterface failed[%x]", __PRETTY_FUNCTION__, __LINE__, err);
                continue;
            }
            assert(s_this->m_d3dVideoContext);
            success = true;
            break;
        } while (true);

        if (!success)
            break;

        s_this->m_thread = std::thread(std::bind(&ScreenCapturer::exec, s_this));
        return true;
    } while (0);

    delete s_this;
    s_this = nullptr;
    return false;
}

void ScreenCapturer::release()
{
    delete s_this;
    s_this = nullptr;
}

void ScreenCapturer::exit()
{
    m_exit = true;
}

void ScreenCapturer::waitForExited()
{
    if (m_thread.joinable())
        m_thread.join();
}

ScreenCapturer::~ScreenCapturer()
{
    exit();
    wakeup();
    waitForExited();
    cleanup();
}

void ScreenCapturer::wakeup()
{
    std::unique_lock g(m_mutex);
    m_cv.notify_all();
}

ScreenCapturer::ScreenCapturer()
    :   ScreenCapturerBase()
{

}

void ScreenCapturer::cleanup()
{
    SAFE_RELEASE(m_d3dDevice)
    SAFE_RELEASE(m_d3dContext)
    SAFE_RELEASE(m_dxgiDevice)
    SAFE_RELEASE(m_dxgiAdapter)
    SAFE_RELEASE(m_dxgiOutput)
    SAFE_RELEASE(m_dxgiOutput1)

    SAFE_RELEASE(m_d3dVideoDevice)
    SAFE_RELEASE(m_d3dVideoContext)
    SAFE_RELEASE(m_d3dVideoProcessor)
    SAFE_RELEASE(m_d3dVideoProcessorEnumerator)
    SAFE_RELEASE(m_d3dVideoProcessorOutputView)
    SAFE_RELEASE(m_d3d11TextureEncoder)
    SAFE_RELEASE(m_d3d11TextureDup)
}

void ScreenCapturer::exec()
{
    HRESULT err;
    DXGI_OUTDUPL_FRAME_INFO infoFrame;
    IDXGIResource *resourceDesktop;
    IDXGIOutputDuplication *dxgiOutputDup = nullptr;
    DXGI_SURFACE_DESC surfaceDesc;
    DXGI_MAPPED_RECT rect;

    while (!m_exit)
    {
        long long t0 = QDateTime::currentMSecsSinceEpoch();
        bool good = false;
        do
        {
            if (dxgiOutputDup == nullptr)
            {
                err = m_dxgiOutput1->DuplicateOutput(m_d3dDevice, &dxgiOutputDup);
                if (FAILED(err))
                {
                     Logger::error("%s:%d - dxgiOutput1->DuplicateOutput failed %x", __PRETTY_FUNCTION__, __LINE__, err);
                }
                break;
            }
            err = dxgiOutputDup->AcquireNextFrame(0, &infoFrame, &resourceDesktop);
            if (FAILED(err))
            {
                if (err == DXGI_ERROR_WAIT_TIMEOUT) // the screen doesn't change
                {
                    if (m_forKeyFrame > 0)
                        this->pushScreenFrame(m_frameCount++, surfaceDesc.Width, surfaceDesc.Height, rect.pBits, rect.Pitch);

                    good = true;
                    break;
                }

                if (err == DXGI_ERROR_ACCESS_LOST)
                {
                    SAFE_RELEASE(dxgiOutputDup)
                    break;
                }

                Logger::error("%s:%d - dxgiOutputDup->AcquireNextFrame failed %x", __PRETTY_FUNCTION__, __LINE__, err);
                break;
            }

            ID3D11Texture2D *d3d11Texture;
            err = resourceDesktop->QueryInterface(__uuidof(ID3D11Texture2D), (void **)(&d3d11Texture));
            resourceDesktop->Release();
            if (FAILED(err))
            {
                Logger::error("%s:%d -  resourceDesktop->QueryInterface failed %x", __PRETTY_FUNCTION__, __LINE__, err);
                break;
            }

            D3D11_TEXTURE2D_DESC  desc = {0};
            d3d11Texture->GetDesc(&desc);

            if (m_w != desc.Width || m_h != desc.Height)
            {
                SAFE_RELEASE(m_d3dVideoProcessor);
                SAFE_RELEASE(m_d3dVideoProcessorEnumerator);
                SAFE_RELEASE(m_d3d11TextureEncoder);
                SAFE_RELEASE(m_d3d11TextureDup);
                SAFE_RELEASE(m_d3dVideoProcessorOutputView);

                bool success = false;
                do
                {
                    D3D11_TEXTURE2D_DESC desc1;
                    ZeroMemory(&desc1, sizeof(D3D11_TEXTURE2D_DESC));
                    desc1.Width = desc.Width;
                    desc1.Height = desc.Height;
                    desc1.MipLevels = 1;
                    desc1.ArraySize = 1;
                    desc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                    desc1.SampleDesc.Count = 1;
                    desc1.Usage = D3D11_USAGE_DEFAULT;
                    desc1.BindFlags = D3D11_BIND_RENDER_TARGET;
                    desc1.CPUAccessFlags = 0;
                    err = m_d3dDevice->CreateTexture2D(&desc1, NULL, &m_d3d11TextureEncoder);
                    if (FAILED(err))
                    {
                        Logger::error("%s:%d -  m_d3dDevice->CreateTexture2D failed %x", __PRETTY_FUNCTION__, __LINE__, err);
                        break;
                    }

                    desc1.Usage = D3D11_USAGE_STAGING;
                    desc1.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                    desc1.BindFlags = 0;
                    desc1.MiscFlags = 0;
                    desc1.MipLevels = 1;
                    desc1.ArraySize = 1;
                    desc1.SampleDesc.Count = 1;
                    err = m_d3dDevice->CreateTexture2D(&desc1, NULL, &m_d3d11TextureDup);
                    if (err != S_OK)
                    {
                        Logger::error("%s:%d -  m_d3dDevice->CreateTexture2D failed %x", __PRETTY_FUNCTION__, __LINE__, err);
                        break;
                    }


                    D3D11_VIDEO_PROCESSOR_CONTENT_DESC contentDesc =
                    {
                        D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE,
                        { 1, 1 }, desc.Width, desc.Height,
                        { 1, 1 }, desc.Width, desc.Height,
                        D3D11_VIDEO_USAGE_PLAYBACK_NORMAL
                    };
                    err = m_d3dVideoDevice->CreateVideoProcessorEnumerator(&contentDesc, &m_d3dVideoProcessorEnumerator);;
                    if (FAILED(err))
                    {
                        Logger::error("%s:%d -  m_d3dVideoDevice->CreateVideoProcessorEnumerator failed %x", __PRETTY_FUNCTION__, __LINE__, err);
                        break;
                    }

                    err = m_d3dVideoDevice->CreateVideoProcessor(m_d3dVideoProcessorEnumerator, 0, &m_d3dVideoProcessor);;
                    if (FAILED(err))
                    {
                        Logger::error("%s:%d -  m_d3dVideoDevice->CreateVideoProcessor failed %x", __PRETTY_FUNCTION__, __LINE__, err);
                        break;
                    }


                    D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC ovD = { D3D11_VPOV_DIMENSION_TEXTURE2D };
                    err = m_d3dVideoDevice->CreateVideoProcessorOutputView(m_d3d11TextureEncoder, m_d3dVideoProcessorEnumerator, &ovD, &m_d3dVideoProcessorOutputView);
                    if (FAILED(err))
                    {
                        Logger::error("%s:%d -  m_d3dVideoDevice->CreateVideoProcessorOutputView failed [%x]", __PRETTY_FUNCTION__, __LINE__, err);
                        break;
                    }

                    success = true;
                } while (0);

                if (!success)
                {
                    SAFE_RELEASE(m_d3dVideoProcessor);
                    SAFE_RELEASE(m_d3dVideoProcessorEnumerator);
                    SAFE_RELEASE(m_d3d11TextureEncoder);
                    SAFE_RELEASE(m_d3d11TextureDup);
                    SAFE_RELEASE(m_d3dVideoProcessorOutputView);
                }
            }

            convertTexture(d3d11Texture, m_d3d11TextureEncoder);
            d3d11Texture->Release();
            dxgiOutputDup->ReleaseFrame();

            m_d3dContext->CopyResource(m_d3d11TextureDup, m_d3d11TextureEncoder);

            IDXGISurface *dxgiSurface;
            err = m_d3d11TextureDup->QueryInterface(__uuidof(IDXGISurface), (void**)&dxgiSurface);

            if (FAILED(err))
            {
                Logger::error("%s:%d -  d3d11TextureBackup->QueryInterface failed %x", __PRETTY_FUNCTION__, __LINE__, err);
                break;
            }



            dxgiSurface->GetDesc(&surfaceDesc);
            err = dxgiSurface->Map(&rect, DXGI_MAP_READ);
            if (FAILED(err))
            {
                Logger::error("%s:%d - dxgiSurface->Map failed %x", __PRETTY_FUNCTION__, __LINE__, err);
                break;
            }

            this->pushScreenFrame(m_frameCount++, surfaceDesc.Width, surfaceDesc.Height, rect.pBits, rect.Pitch);

            dxgiSurface->Unmap();
            dxgiSurface->Release();

            good = true;
        } while (0);

        long long t1 = QDateTime::currentMSecsSinceEpoch();
        int r = 30 - (t1 - t0);
        if (r <= 0)
            r = 1;
        if (!good)
        {
            r = 500;
        }

        std::unique_lock g(m_mutex);
        m_cv.wait_for(g, std::chrono::milliseconds(r));
    }

    SAFE_RELEASE(dxgiOutputDup)
}

bool ScreenCapturer::convertTexture(ID3D11Texture2D *src, ID3D11Texture2D *dst)
{
    D3D11_TEXTURE2D_DESC inDesc = { 0 };
    D3D11_TEXTURE2D_DESC outDesc = { 0 };
    src->GetDesc(&inDesc);
    dst->GetDesc(&outDesc);

    ID3D11VideoProcessorInputView* videoProcessorInputView = nullptr;
    D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputVD = { 0, D3D11_VPIV_DIMENSION_TEXTURE2D,{ 0,0 } };
    HRESULT err = m_d3dVideoDevice->CreateVideoProcessorInputView(src, m_d3dVideoProcessorEnumerator, &inputVD, &videoProcessorInputView);
    if (FAILED(err))
    {
        Logger::error("%s:%d - m_d3dVideoDevice->CreateVideoProcessorInputView failed %x", __PRETTY_FUNCTION__, __LINE__, err);
        return false;
    }

    D3D11_VIDEO_PROCESSOR_STREAM stream = { TRUE, 0, 0, 0, 0, nullptr, videoProcessorInputView, nullptr };
    err = m_d3dVideoContext->VideoProcessorBlt(m_d3dVideoProcessor, m_d3dVideoProcessorOutputView, 0, 1, &stream);
    if (FAILED(err))
    {
        SAFE_RELEASE(videoProcessorInputView);
        Logger::error("%s:%d - m_d3dVideoContext->VideoProcessorBlt failed %x", __PRETTY_FUNCTION__, __LINE__, err);
        return false;
    }
    SAFE_RELEASE(videoProcessorInputView);
    return true;
}

void ScreenCapturer::fillPlanes(unsigned w, unsigned h, const unsigned char* src, size_t pitch,
                                unsigned char* dstY, unsigned char* dstU, unsigned char* dstV, size_t dstStride)
{
    libyuv::ABGRToI420(src, pitch, dstY, dstStride, dstU, dstStride, dstV, dstStride, w, h);
}

#endif
