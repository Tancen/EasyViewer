#include <QtGlobal>

#ifdef Q_OS_LINUX

#ifndef SCREENCAPTURER_H
#define SCREENCAPTURER_H

#include "ScreenCapturerBase.h"
#include <thread>
#include <condition_variable>

class ScreenCapturer : public ScreenCapturerBase
{
public:
    static ScreenCapturer* share();
    static bool init();
    static void release();
    void exit();
    void waitForExited();

    ~ScreenCapturer();

    void setCursorPosition(int x, int y);
    void mousePress(unsigned int button, int x, int y);
    void mouseRelease(unsigned int button, int x, int y);
    void mouseDoubleClick(unsigned int button, int x, int y);
    void mouseWheel(int delta);
    void keyPress(unsigned int k);
    void keyRelease(unsigned int k);

    void wakeup();

private:
    ScreenCapturer();
    void exec();

    void fillPlanes(unsigned w, unsigned h, const unsigned char* src, size_t pitch,
                    unsigned char* dstY, unsigned char* dstU, unsigned char* dstV, size_t dstStride);

private:
    void *m_display = nullptr;
    void *m_screen = nullptr;
    unsigned long m_window = 0;

    int m_xCursorPosition = -1;
    int m_yCursorPosition = -1;

    int64_t m_frameCount = 0;

    bool m_exit = false;
    std::thread m_thread;

    std::mutex m_mutex;
    std::condition_variable m_cv;

    static ScreenCapturer* s_this;
};

#endif // SCREENCAPTURER_H

#endif
