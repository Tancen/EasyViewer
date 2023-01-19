#include <QtGlobal>

#ifdef Q_OS_WIN

#ifndef SCREENCAPTURER_H
#define SCREENCAPTURER_H

#include "ScreenCapturerBase.h"
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <thread>
#include <mutex>

class ScreenCapturer : public ScreenCapturerBase
{
public:
    static ScreenCapturer* share();
    static bool init();
    static void release();
    void exit();
    void waitForExited();

    ~ScreenCapturer();

    void wakeup();

private:
    ScreenCapturer();

    void cleanup();

    void exec();

    bool convertTexture(ID3D11Texture2D* src, ID3D11Texture2D*dst);
    void fillPlanes(unsigned w, unsigned h, const unsigned char* src, size_t pitch,
                    unsigned char* dstY, unsigned char* dstU, unsigned char* dstV, size_t dstStride);

private:
    ID3D11Device *m_d3dDevice = nullptr;
    ID3D11DeviceContext *m_d3dContext = nullptr;
    IDXGIDevice *m_dxgiDevice = nullptr;
    IDXGIAdapter *m_dxgiAdapter = nullptr;
    IDXGIOutput *m_dxgiOutput = nullptr;
    IDXGIOutput1 *m_dxgiOutput1 = nullptr;

    ID3D11VideoDevice *m_d3dVideoDevice = nullptr;
    ID3D11VideoContext *m_d3dVideoContext = nullptr;
    ID3D11VideoProcessor *m_d3dVideoProcessor = nullptr;
    ID3D11VideoProcessorEnumerator* m_d3dVideoProcessorEnumerator = nullptr;
    ID3D11VideoProcessorOutputView *m_d3dVideoProcessorOutputView = nullptr;

    ID3D11Texture2D *m_d3d11TextureEncoder = nullptr;
    ID3D11Texture2D *m_d3d11TextureDup = nullptr;

    int m_xCursorPosition = -1;
    int m_yCursorPosition = -1;

    bool m_exit = false;
    std::thread m_thread;
    int64_t m_frameCount = 0;
    std::mutex m_mutex;
    std::condition_variable m_cv;

    static ScreenCapturer* s_this;
};

#endif

#endif // SCREENCAPTURER_H
