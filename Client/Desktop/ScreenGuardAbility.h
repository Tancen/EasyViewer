#ifndef SCREENGUARDABILITY_H
#define SCREENGUARDABILITY_H

#include "Guard.h"
#include <Qt>

class ScreenGuardAbility  : public Guard::IAbility
{
public:
    ScreenGuardAbility();
    ~ScreenGuardAbility();

    int role() override;

private:
    bool handleCompleteData(unsigned tag, const unsigned char* data, size_t len ) override;
    void whenNewScreenFrame(int64_t seq, unsigned w, unsigned h, const unsigned char* data, size_t len);

    void handleRequestOfSubscribeScreen(const unsigned char* data, size_t len);
    void handleCommandOfMouseMove(const unsigned char* data, size_t len);
    void handleCommandOfKeyPress(const unsigned char* data, size_t len);
    void handleCommandOfKeyRelease(const unsigned char* data, size_t len);
    void handleCommandOfMouseButtonPress(const unsigned char* data, size_t len);
    void handleCommandOfMouseButtonRelease(const unsigned char* data, size_t len);
    void handleCommandOfMouseButtonDoubleClick(const unsigned char* data, size_t len);
    void handleCommandOfMouseWheel(const unsigned char* data, size_t len);
    void handleCommandOfSetClipboardText(const unsigned char* data, size_t len);
    void handleCommandOfSetAutoPublishDesktop(const unsigned char* data, size_t len);
    void handleCommandOfShortcut(const unsigned char* data, size_t len);

    void publishScreenFrame(User::ID userId, int64_t seq, unsigned w, unsigned h, const unsigned char* data, size_t len);
    void publishClipboardText(const std::string& text);

    void handleLoggedinEvent() override;
    void handleConnectionLostEvent() override;

    void pressKey(Qt::Key key);
    void releaseKey(Qt::Key key);

private:
    std::recursive_mutex m_mutex;

    std::unique_ptr<Cryptology::AES<EasyIO::ByteBuffer>> m_aesDirect;
    std::set<User::ID> m_subscribers;

    static const std::map<int, int> KEY_MAP;
};

#endif // SCREENGUARDABILITY_H
