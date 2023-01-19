#include <QtGlobal>
#ifdef Q_OS_LINUX

#include <QDateTime>
#include <QCursor>
#include "ScreenCapturer_linux.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Define.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xmd.h>
#include <X11/Xatom.h>
#include <X11/Xdefs.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XTest.h>   //libxtst-dev
#include <cstring>
#include "libyuv.h"
#include "Buttons_linux.h"



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
        s_this->m_display = XOpenDisplay(nullptr);
        if (!s_this->m_display)
        {
            Logger::error("%s:%d - XOpenDisplay failed", __PRETTY_FUNCTION__, __LINE__);
            break;
        }

        s_this->m_screen = XDefaultScreenOfDisplay((Display*)s_this->m_display);
        if (!s_this->m_screen)
        {
            Logger::error("%s:%d - XDefaultScreenOfDisplay failed", __PRETTY_FUNCTION__, __LINE__);
            break;
        }

        s_this->m_window = XDefaultRootWindow((Display*)s_this->m_display);
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
    if (m_display)
        XCloseDisplay((Display*)m_display);
}

void ScreenCapturer::setCursorPosition(int x, int y)
{
    m_lock.lock();
    XTestFakeMotionEvent((Display*)m_display, 0, x, y, 0);
    XFlush((Display*)m_display);
    m_lock.unlock();
}

void ScreenCapturer::mousePress(unsigned int button, int x, int y)
{
    m_lock.lock();
    setCursorPosition(x, y);
    XTestFakeButtonEvent((Display*)m_display, button, true, 0);
    XFlush((Display*)m_display);
    m_lock.unlock();
}

void ScreenCapturer::mouseRelease(unsigned int button, int x, int y)
{
    m_lock.lock();
    setCursorPosition(x, y);
    XTestFakeButtonEvent((Display*)m_display, button, false, 0);
    XFlush((Display*)m_display);
    m_lock.unlock();
}

void ScreenCapturer::mouseDoubleClick(unsigned int button, int x, int y)
{
    m_lock.lock();
    setCursorPosition(x, y);
    XTestFakeButtonEvent((Display*)m_display, button, true, 0);
    XTestFakeButtonEvent((Display*)m_display, button, false, 0);
    XTestFakeButtonEvent((Display*)m_display, button, true, 0);
    XTestFakeButtonEvent((Display*)m_display, button, false, 0);
    XFlush((Display*)m_display);
    m_lock.unlock();
}

void ScreenCapturer::mouseWheel(int delta)
{
    m_lock.lock();
    if (delta > 0)
    {
        XTestFakeButtonEvent((Display*)m_display, SCROLL_WHEEL_UP, true, 0);
        XTestFakeButtonEvent((Display*)m_display, SCROLL_WHEEL_UP, false, 0);
    }
    else
    {
        XTestFakeButtonEvent((Display*)m_display, SCROLL_WHEEL_DOWN, true, 0);
        XTestFakeButtonEvent((Display*)m_display, SCROLL_WHEEL_DOWN, false, 0);
    }

    m_lock.unlock();
}

void ScreenCapturer::keyPress(unsigned int k)
{
    std::string err;
    m_lock.lock();
    XTestFakeKeyEvent((Display*)m_display, k, true, 0);
    XFlush((Display*)m_display);
    m_lock.unlock();
}

void ScreenCapturer::keyRelease(unsigned int k)
{
    std::string err;
    m_lock.lock();
    XTestFakeKeyEvent((Display*)m_display, k, false, 0);
    XFlush((Display*)m_display);
    m_lock.unlock();
}

void ScreenCapturer::wakeup()
{
    std::unique_lock g(m_mutex);
        m_cv.notify_all();
}

ScreenCapturer::ScreenCapturer()
    :   ScreenCapturerBase(),
        m_display(nullptr),
        m_screen(nullptr),
        m_xCursorPosition(-1),
        m_yCursorPosition(-1),
        m_exit(false)
{

}

void ScreenCapturer::exec()
{
    while (!m_exit)
    {
        long long t0 = QDateTime::currentMSecsSinceEpoch();
        {
            do
            {
                std::string err;
                m_lock.lock();
                XWindowAttributes attr;
                XGetWindowAttributes((Display*)m_display, m_window, &attr);
                XImage *xImage = XGetImage((Display*)m_display, m_window, 0, 0,
                                          attr.width, attr.height,
                                          AllPlanes,
                                          ZPixmap
                                          );
                m_lock.unlock();
                if (!xImage)
                {
                    Logger::error("%s:%d - XGetImage failed", __PRETTY_FUNCTION__, __LINE__);
                    break;
                }

                if (xImage->byte_order != LSBFirst
                    || xImage->bits_per_pixel != 32
                    || xImage->depth != 24)
                {
                    Logger::error("%s:%d - xImage->byte_order != LSBFirst || xImage->bits_per_pixel != 32 "
                                  " || xImage->depth != 24", __PRETTY_FUNCTION__, __LINE__);
                    XDestroyImage(xImage);
                    return;
                }

                this->pushScreenFrame(m_frameCount++, xImage->width, xImage->height, (unsigned char*)xImage->data, xImage->bytes_per_line);

                XDestroyImage(xImage);

            } while (0);
        }

        long long t1 = QDateTime::currentMSecsSinceEpoch();
        int r = 30 - (t1 - t0);
        if (r <= 0)
            r = 1;

        std::unique_lock g(m_mutex);
        m_cv.wait_for(g, std::chrono::milliseconds(r));
    }
}


void ScreenCapturer::fillPlanes(unsigned w, unsigned h, const unsigned char* src, size_t pitch,
                                unsigned char* dstY, unsigned char* dstU, unsigned char* dstV, size_t dstStride)
{
    libyuv::ARGBToI420(src, pitch, dstY, dstStride, dstU, dstStride, dstV, dstStride, w, h);
}

#endif
