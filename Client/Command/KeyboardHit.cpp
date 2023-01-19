#include <QtGlobal>
#include "KeyboardHit.h"

#ifdef Q_OS_WIN
    #include <Windows.h>
    #include <conio.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <signal.h>
#endif
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <QString>
#include <Global/Component/Logger/Logger.h>

KeyboardHit* KeyboardHit::s_this = nullptr;

KeyboardHit *KeyboardHit::share()
{
    return s_this;
}

bool KeyboardHit::init()
{
    if (!s_this)
    {
#ifndef Q_OS_WIN
        struct sigaction act;
        act.sa_handler = [](int){
            s_this->publishResizeEvent();
        };
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        if(sigaction(SIGWINCH, &act, NULL))
        {
            return false;
        }
#endif

        s_this = new KeyboardHit();
    }

    return true;
}

void KeyboardHit::release()
{
    delete s_this;
    s_this = nullptr;
}

void KeyboardHit::exit()
{
    m_exit = true;
}

void KeyboardHit::waitForExited()
{
    if (m_thread.joinable())
        m_thread.join();
}

void KeyboardHit::subscribe(void *key, KeyboardHit::HitCallback hitCallback, ResizeCallback resizeCallback)
{
    assert(hitCallback);
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    m_followers.insert({key, {hitCallback, resizeCallback}});
}

void KeyboardHit::unsubscribe(void *key)
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    m_followers.erase(key);
}

KeyboardHit::KeyboardHit()
    :   m_exit(false)
{
    m_thread = std::thread(std::bind(&KeyboardHit::exec, this));
}

KeyboardHit::~KeyboardHit()
{
#ifndef Q_OS_WIN
    struct sigaction act;
    act.sa_handler = [](int){};
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGWINCH, &act, NULL);
#endif
}

void KeyboardHit::exec()
{
    const int BUF_LEN   = 512;
    char buf[BUF_LEN];

#ifdef Q_OS_WIN
    const int MAX_EVENT = BUF_LEN;
    INPUT_RECORD events[MAX_EVENT];
    auto hStdInput = GetStdHandle(STD_INPUT_HANDLE);
#else
    int nfds = STDIN_FILENO + 1;
    timeval val;
    val.tv_sec = 0;
    val.tv_usec = 100000;
#endif

    while (!m_exit)
    {
#ifdef Q_OS_WIN
        DWORD nEvent;
        bool success = GetNumberOfConsoleInputEvents(hStdInput, &nEvent);
        assert(success);
        if (nEvent)
        {
            success = ReadConsoleInput(hStdInput, events, MAX_EVENT, &nEvent);
            assert(success);

            int len = 0;
            for (int i = 0; i < nEvent; i++)
            {
                INPUT_RECORD& e = events[i];
                if (e.EventType == KEY_EVENT && e.Event.KeyEvent.bKeyDown == true && e.Event.KeyEvent.uChar.UnicodeChar != 0)
                {
                    if (e.Event.KeyEvent.uChar.UnicodeChar == L'\r')
                    {
                        buf[len++] = '\n';
                    }
                    else
                    {
                        auto bytes = QString::fromUtf16((char16_t*)&e.Event.KeyEvent.uChar.UnicodeChar, 1).toUtf8();
                        memcpy(buf + len, bytes.data(), bytes.size());
                        len += bytes.size();
                    }
                }
                else if (e.EventType == WINDOW_BUFFER_SIZE_EVENT)
                {
                    publishResizeEvent();
                }
            }

            if (len)
            {
                std::lock_guard<std::recursive_mutex> guard(m_mutex);
                for (auto it : m_followers)
                    if (it.second.hitCallback)
                        it.second.hitCallback(buf, len);
            }
        }
#else
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(STDIN_FILENO, &rset);

        int ret = select(nfds, &rset, NULL, NULL, &val);
        if (ret < 0)
        {
            Logger::error("select failed, err[%d]: %s", ret, strerror(errno));
            return;
        }

        if (ret != 0)
        {
            if (FD_ISSET(STDIN_FILENO, &rset))
            {
                int n = read(STDIN_FILENO, buf, BUF_LEN);
                if (n < 0)
                {
                    Logger::error("read kbin failed, err[%d]: %s", ret, strerror(errno));
                    return;
                }

                if (n == 0)
                {
                    Logger::error("read n == 0");
                    break;
                }

                {
                    std::lock_guard<std::recursive_mutex> guard(m_mutex);
                    for (auto it : m_followers)
                    {
                        if (it.second.hitCallback)
                            it.second.hitCallback(buf, n);
                    }
                }
            }
        }
#endif
    }
}

void KeyboardHit::publishResizeEvent()
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    for (auto it : m_followers)
        if (it.second.resizeCallback)
            it.second.resizeCallback();
}
