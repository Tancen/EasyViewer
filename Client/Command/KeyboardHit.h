#ifndef KEYBOARDHIT_H
#define KEYBOARDHIT_H

#include <functional>
#include <thread>
#include <mutex>
#include <map>

class KeyboardHit
{
public:
    typedef std::function<void(const char* hits, size_t len)> HitCallback;
    typedef std::function<void()> ResizeCallback;
public:
    static KeyboardHit* share();
    static bool init();
    static void release();

    void exit();
    void waitForExited();

    void subscribe(void* key, HitCallback hitCallback, ResizeCallback resizeCallback);
    void unsubscribe(void* key);

private:
    KeyboardHit();
    ~KeyboardHit();

    void exec();

    void publishResizeEvent();

private:
    struct Callbacks
    {
        HitCallback hitCallback;
        ResizeCallback resizeCallback;
    };

private:
    bool m_exit;
    std::map<void*, Callbacks> m_followers;

    std::recursive_mutex m_mutex;
    std::thread m_thread;

    static KeyboardHit* s_this;
};

#endif // KEYBOARDHIT_H
