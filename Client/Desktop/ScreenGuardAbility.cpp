#include "ScreenGuardAbility.h"
#include <QtGlobal>
#include "Global/Define.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/Protocol.h"

#ifdef Q_OS_WIN
    #include "Desktop/ScreenCapturer_win.h"
#else
    #include "Desktop/ScreenCapturer_linux.h"
    #include "Buttons_linux.h"
#endif

#include "Global/Protocol/Screen/Screen.pb.h"
#include "Global/Protocol/Screen/Control.pb.h"
#include "Global/Protocol/Error.h"
#include "AuthChecker.h"
#include <cinttypes>
#include <zlib.h>
#include <zconf.h>
#include "Clipboard.h"
#include "Shortcuts.h"
#include "UsualParseProtobufDataMacro.h"

#ifdef Q_OS_WIN
const std::map<int, int> ScreenGuardAbility::KEY_MAP = []()
{
    //https://docs.microsoft.com/zh-cn/windows/win32/inputdev/virtual-key-codes
    static std::map<int, int> ret;
    ret.insert({Qt::Key_Backspace, VK_BACK});
    ret.insert({Qt::Key_Tab, VK_TAB});
    ret.insert({Qt::Key_Return, VK_RETURN});
    ret.insert({Qt::Key_Enter, VK_RETURN});
    ret.insert({Qt::Key_Shift, VK_SHIFT});
    ret.insert({Qt::Key_Control, VK_CONTROL});
    ret.insert({Qt::Key_Alt, VK_MENU});
    ret.insert({Qt::Key_CapsLock, VK_CAPITAL});
    ret.insert({Qt::Key_Escape, VK_ESCAPE});
    ret.insert({Qt::Key_Space, VK_SPACE});
    ret.insert({Qt::Key_PageUp, VK_PRIOR});
    ret.insert({Qt::Key_PageDown, VK_NEXT});
    ret.insert({Qt::Key_End, VK_END});
    ret.insert({Qt::Key_Home, VK_HOME});
    ret.insert({Qt::Key_Left, VK_LEFT});
    ret.insert({Qt::Key_Up, VK_UP});
    ret.insert({Qt::Key_Right, VK_RIGHT});
    ret.insert({Qt::Key_Down, VK_DOWN});
    ret.insert({Qt::Key_Insert, VK_INSERT});
    ret.insert({Qt::Key_Delete, VK_DELETE});
    ret.insert({Qt::Key_Print, VK_SNAPSHOT});
    ret.insert({Qt::Key_0, 0x30});
    ret.insert({Qt::Key_ParenRight, 0x30});
    ret.insert({Qt::Key_1, 0x31});
    ret.insert({Qt::Key_Exclam, 0x31});
    ret.insert({Qt::Key_2, 0x32});
    ret.insert({Qt::Key_At, 0x32});
    ret.insert({Qt::Key_3, 0x33});
    ret.insert({Qt::Key_NumberSign, 0x33});
    ret.insert({Qt::Key_4, 0x34});
    ret.insert({Qt::Key_Dollar, 0x34});
    ret.insert({Qt::Key_5, 0x35});
    ret.insert({Qt::Key_Percent, 0x35});
    ret.insert({Qt::Key_6, 0x36});
    ret.insert({Qt::Key_AsciiCircum, 0x36});
    ret.insert({Qt::Key_7, 0x37});
    ret.insert({Qt::Key_Ampersand, 0x37});
    ret.insert({Qt::Key_8, 0x38});
    ret.insert({Qt::Key_Asterisk, 0x38});
    ret.insert({Qt::Key_9, 0x39});
    ret.insert({Qt::Key_ParenLeft, 0x39});
    ret.insert({Qt::Key_A, 0x41});
    ret.insert({Qt::Key_B, 0x42});
    ret.insert({Qt::Key_C, 0x43});
    ret.insert({Qt::Key_D, 0x44});
    ret.insert({Qt::Key_E, 0x45});
    ret.insert({Qt::Key_F, 0x46});
    ret.insert({Qt::Key_G, 0x47});
    ret.insert({Qt::Key_H, 0x48});
    ret.insert({Qt::Key_I, 0x49});
    ret.insert({Qt::Key_J, 0x4A});
    ret.insert({Qt::Key_K, 0x4B});
    ret.insert({Qt::Key_L, 0x4C});
    ret.insert({Qt::Key_M, 0x4D});
    ret.insert({Qt::Key_N, 0x4E});
    ret.insert({Qt::Key_O, 0x4F});
    ret.insert({Qt::Key_P, 0x50});
    ret.insert({Qt::Key_Q, 0x51});
    ret.insert({Qt::Key_R, 0x52});
    ret.insert({Qt::Key_S, 0x53});
    ret.insert({Qt::Key_T, 0x54});
    ret.insert({Qt::Key_U, 0x55});
    ret.insert({Qt::Key_V, 0x56});
    ret.insert({Qt::Key_W, 0x57});
    ret.insert({Qt::Key_X, 0x58});
    ret.insert({Qt::Key_Y, 0x59});
    ret.insert({Qt::Key_Z, 0x5A});

    ret.insert({Qt::Key_F1, VK_F1});
    ret.insert({Qt::Key_F2, VK_F2});
    ret.insert({Qt::Key_F3, VK_F3});
    ret.insert({Qt::Key_F4, VK_F4});
    ret.insert({Qt::Key_F5, VK_F5});
    ret.insert({Qt::Key_F6, VK_F6});
    ret.insert({Qt::Key_F7, VK_F7});
    ret.insert({Qt::Key_F8, VK_F8});
    ret.insert({Qt::Key_F9, VK_F9});
    ret.insert({Qt::Key_F10, VK_F10});
    ret.insert({Qt::Key_F11, VK_F11});
    ret.insert({Qt::Key_F12, VK_F12});

    ret.insert({Qt::Key_NumLock, VK_NUMLOCK});

    ret.insert({Qt::Key_Equal, VK_OEM_PLUS});
    ret.insert({Qt::Key_Plus, VK_OEM_PLUS});
    ret.insert({Qt::Key_Comma, VK_OEM_COMMA});
    ret.insert({Qt::Key_Less, VK_OEM_COMMA});
    ret.insert({Qt::Key_Minus, VK_OEM_MINUS});
    ret.insert({Qt::Key_Underscore, VK_OEM_MINUS});
    ret.insert({Qt::Key_Period, VK_DECIMAL});
    ret.insert({Qt::Key_Greater, VK_DECIMAL});

    ret.insert({Qt::Key_Semicolon, VK_OEM_1});
    ret.insert({Qt::Key_Colon, VK_OEM_1});
    ret.insert({Qt::Key_Slash, VK_OEM_2});
    ret.insert({Qt::Key_Question, VK_OEM_2});
    ret.insert({Qt::Key_QuoteLeft, VK_OEM_3});
    ret.insert({Qt::Key_AsciiTilde, VK_OEM_3});
    ret.insert({Qt::Key_BracketLeft, VK_OEM_4});
    ret.insert({Qt::Key_BraceLeft, VK_OEM_4});
    ret.insert({Qt::Key_Backslash, VK_OEM_5});
    ret.insert({Qt::Key_Bar, VK_OEM_5});
    ret.insert({Qt::Key_BracketRight, VK_OEM_6});
    ret.insert({Qt::Key_BraceRight, VK_OEM_6});
    ret.insert({Qt::Key_Apostrophe, VK_OEM_7});
    ret.insert({Qt::Key_QuoteDbl, VK_OEM_7});

    return ret;
}();
#else
const std::map<int, int> ScreenGuardAbility::KEY_MAP = []()
{
    // type 'xev' in terminal, can get the key code you wants.
    static std::map<int, int> ret;
    ret.insert({Qt::Key_Backspace, 22});
    ret.insert({Qt::Key_Tab, 23});
    ret.insert({Qt::Key_Return, 36});
    ret.insert({Qt::Key_Enter, 36});
    ret.insert({Qt::Key_Shift, 50});
    ret.insert({Qt::Key_Control, 37});
    ret.insert({Qt::Key_Alt, 64});
    ret.insert({Qt::Key_CapsLock, 66});
    ret.insert({Qt::Key_Escape, 9});
    ret.insert({Qt::Key_Space, 65});
    ret.insert({Qt::Key_PageUp, 112});
    ret.insert({Qt::Key_PageDown, 117});
    ret.insert({Qt::Key_End, 115});
    ret.insert({Qt::Key_Home, 110});
    ret.insert({Qt::Key_Left, 113});
    ret.insert({Qt::Key_Up, 111});
    ret.insert({Qt::Key_Right, 114});
    ret.insert({Qt::Key_Down, 116});
    ret.insert({Qt::Key_Insert, 118});
    ret.insert({Qt::Key_Delete, 119});
    ret.insert({Qt::Key_Print, 107});
    ret.insert({Qt::Key_0, 19});
    ret.insert({Qt::Key_ParenRight, 19});
    ret.insert({Qt::Key_1, 10});
    ret.insert({Qt::Key_Exclam, 10});
    ret.insert({Qt::Key_2, 11});
    ret.insert({Qt::Key_At, 11});
    ret.insert({Qt::Key_3, 12});
    ret.insert({Qt::Key_NumberSign, 12});
    ret.insert({Qt::Key_4, 13});
    ret.insert({Qt::Key_Dollar, 13});
    ret.insert({Qt::Key_5, 14});
    ret.insert({Qt::Key_Percent, 14});
    ret.insert({Qt::Key_6, 15});
    ret.insert({Qt::Key_AsciiCircum, 15});
    ret.insert({Qt::Key_7, 16});
    ret.insert({Qt::Key_Ampersand, 16});
    ret.insert({Qt::Key_8, 17});
    ret.insert({Qt::Key_Asterisk, 63});
    ret.insert({Qt::Key_9, 18});
    ret.insert({Qt::Key_ParenLeft, 18});
    ret.insert({Qt::Key_A, 38});
    ret.insert({Qt::Key_B, 56});
    ret.insert({Qt::Key_C, 54});
    ret.insert({Qt::Key_D, 40});
    ret.insert({Qt::Key_E, 26});
    ret.insert({Qt::Key_F, 41});
    ret.insert({Qt::Key_G, 42});
    ret.insert({Qt::Key_H, 43});
    ret.insert({Qt::Key_I, 31});
    ret.insert({Qt::Key_J, 44});
    ret.insert({Qt::Key_K, 45});
    ret.insert({Qt::Key_L, 46});
    ret.insert({Qt::Key_M, 58});
    ret.insert({Qt::Key_N, 57});
    ret.insert({Qt::Key_O, 32});
    ret.insert({Qt::Key_P, 33});
    ret.insert({Qt::Key_Q, 24});
    ret.insert({Qt::Key_R, 27});
    ret.insert({Qt::Key_S, 39});
    ret.insert({Qt::Key_T, 28});
    ret.insert({Qt::Key_U, 30});
    ret.insert({Qt::Key_V, 55});
    ret.insert({Qt::Key_W, 25});
    ret.insert({Qt::Key_X, 53});
    ret.insert({Qt::Key_Y, 29});
    ret.insert({Qt::Key_Z, 52});

    ret.insert({Qt::Key_F1, 67});
    ret.insert({Qt::Key_F2, 68});
    ret.insert({Qt::Key_F3, 69});
    ret.insert({Qt::Key_F4, 70});
    ret.insert({Qt::Key_F5, 71});
    ret.insert({Qt::Key_F6, 72});
    ret.insert({Qt::Key_F7, 73});
    ret.insert({Qt::Key_F8, 74});
    ret.insert({Qt::Key_F9, 75});
    ret.insert({Qt::Key_F10, 76});
    ret.insert({Qt::Key_F11, 95});
    ret.insert({Qt::Key_F12, 96});

    ret.insert({Qt::Key_NumLock, 77});

    ret.insert({Qt::Key_Equal, 21});
    ret.insert({Qt::Key_Plus, 86});
    ret.insert({Qt::Key_Comma, 59});
    ret.insert({Qt::Key_Less, 59});
    ret.insert({Qt::Key_Minus, 20});
    ret.insert({Qt::Key_Underscore, 20});
    ret.insert({Qt::Key_Period, 60});
    ret.insert({Qt::Key_Greater, 60});

    ret.insert({Qt::Key_Semicolon, 47});
    ret.insert({Qt::Key_Colon, 47});
    ret.insert({Qt::Key_Slash, 61});
    ret.insert({Qt::Key_Question, 61});
    ret.insert({Qt::Key_QuoteLeft, 49});
    ret.insert({Qt::Key_AsciiTilde, 49});
    ret.insert({Qt::Key_BracketLeft, 34});
    ret.insert({Qt::Key_BraceLeft, 34});
    ret.insert({Qt::Key_Backslash, 51});
    ret.insert({Qt::Key_Bar, 51});
    ret.insert({Qt::Key_BracketRight, 35});
    ret.insert({Qt::Key_BraceRight, 35});
    ret.insert({Qt::Key_Apostrophe, 48});
    ret.insert({Qt::Key_QuoteDbl, 48});

    return ret;
}();
#endif


ScreenGuardAbility::ScreenGuardAbility()
{
    if (ScreenCapturer::share())
    {
        ScreenCapturer::share()->addFollower(this,
            std::bind(&ScreenGuardAbility::whenNewScreenFrame, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)
            );
    }

    Clipboard::share()->subscribe(this, std::bind(&ScreenGuardAbility::publishClipboardText, this, std::placeholders::_1));
}

ScreenGuardAbility::~ScreenGuardAbility()
{
    if (ScreenCapturer::share())
    {
        ScreenCapturer::share()->removeFollower(this);
    }

    Clipboard::share()->unsubscribe(this);
}

int ScreenGuardAbility::role()
{
    return GLOBAL_CONNECTION_ROLE_SCREEN;
}

bool ScreenGuardAbility::handleCompleteData(unsigned tag, const unsigned char *data, size_t len)
{
    switch (tag)
    {
    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_SUBSCRIBE_SCREEN2:
        handleRequestOfSubscribeScreen(data, len);
        return true;
    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_MOVE:
        handleCommandOfMouseMove(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_KEY_PRESS:
        handleCommandOfKeyPress(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_KEY_RELEASE:
        handleCommandOfKeyRelease(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_PRESS:
        handleCommandOfMouseButtonPress(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_RELEASE:
        handleCommandOfMouseButtonRelease(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_DOUBLE_CLICK:
        handleCommandOfMouseButtonDoubleClick(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_WHEEL:
        handleCommandOfMouseWheel(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_SET_CLIPBOARD_TEXT:
        handleCommandOfSetClipboardText(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_SET_AUTO_PUBLISH_DESKTOP:
        handleCommandOfSetAutoPublishDesktop(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_SHORTCUT:
        handleCommandOfShortcut(data, len);
        return true;
    }

    return false;
}

void ScreenGuardAbility::whenNewScreenFrame(int64_t seq, unsigned w, unsigned h, const unsigned char* data, size_t len)
{
    publishScreenFrame(this->selfUserId(), seq, w, h, data, len);
}

void ScreenGuardAbility::handleRequestOfSubscribeScreen(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Screen::RequestSubscribeScreen2, request, data, len, aes())

    Global::Protocol::Screen::ResponseSubscribeScreen2 response;
    response.set_async_task_id(request.async_task_id());
    response.set_async_task_certificate(request.async_task_certificate());

    std::string errorString;
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    int err = GLOBAL_PROTOCOL_ERR_NO_ERROR;
    do
    {
        if (!ScreenCapturer::share())
        {
            err = GLOBAL_PROTOCOL_ERR_UNSUPPORTED;
            errorString = Global::Protocol::formatError(err);
            break;
        }

        if(!AuthChecker::share()->test(request.auth_string()))
        {
            err = GLOBAL_PROTOCOL_ERR_PASSWORD_INCORRECT;
            errorString = Global::Protocol::formatError(err);
            break;
        }

        if (m_subscribers.empty())
        {
            m_aesDirect = Cryptology::AES<EasyIO::ByteBuffer>::newInstance(&errorString);
            if (!m_aesDirect.get())
            {
                err = GLOBAL_PROTOCOL_ERR_OPERATE_FAILED;
                break;
            }
        }
        m_subscribers.insert(request.user_id());
        response.set_secret_key(m_aesDirect->key());

    } while(0);


    EasyIO::ByteBuffer buf;
    response.set_err_code(err);
    response.set_err_string(errorString);
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_SUBSCRIBE_SCREEN2, response, buf, aes())

    connection()->send(buf);

    if (err == GLOBAL_PROTOCOL_ERR_NO_ERROR)
        ScreenCapturer::share()->start();
}

void ScreenGuardAbility::handleCommandOfMouseMove(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Screen::ControlMouseMove, request, data, len, m_aesDirect)

#ifdef Q_OS_WIN
    SetCursorPos(request.x(), request.y());
#else
    ScreenCapturer::share()->setCursorPosition(request.x(), request.y());
#endif

    ScreenCapturer::share()->wakeup();
}

void ScreenGuardAbility::handleCommandOfKeyPress(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Screen::ControlKeyPress, request, data, len, m_aesDirect)

    pressKey((Qt::Key)request.key());
    ScreenCapturer::share()->wakeup();
}


void ScreenGuardAbility::handleCommandOfKeyRelease(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Screen::ControlKeyRelease, request, data, len, m_aesDirect)

    releaseKey((Qt::Key)request.key());

    ScreenCapturer::share()->wakeup();
}

void ScreenGuardAbility::handleCommandOfMouseButtonPress(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Screen::ControlMouseButtonPress, request, data, len, m_aesDirect)

    unsigned int bt;
#ifdef Q_OS_WIN
    switch ((Qt::MouseButton)request.button())
    {
    case Qt::LeftButton:
        bt = MOUSEEVENTF_LEFTDOWN;
        break;
    case Qt::RightButton:
        bt = MOUSEEVENTF_RIGHTDOWN;
        break;
    case Qt::MiddleButton:
        bt = MOUSEEVENTF_MIDDLEDOWN;
        break;
    default:
        Logger::warning("%s:%d - unknow button %d, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        (int)request.button(), connection()->peerIP().c_str(), connection()->peerPort());
        return;
    }

    SetCursorPos(request.x(), request.y());
    mouse_event(bt, 0, 0, 0, 0);
#else
    switch ((Qt::MouseButton)request.button())
    {
    case Qt::LeftButton:
        bt = LEFT_BUTTON;
        break;
    case Qt::RightButton:
        bt = RIGHT_BUTTON;
        break;
    case Qt::MiddleButton:
        bt = MID_BUTTON;
        break;
    default:
        Logger::warning("%s:%d - unknow button %d, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        (int)request.button(), connection()->peerIP().c_str(), connection()->peerPort());
        return;
    }

    ScreenCapturer::share()->mousePress(bt, request.x(), request.y());
#endif

    ScreenCapturer::share()->wakeup();
}

void ScreenGuardAbility::handleCommandOfMouseButtonRelease(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Screen::ControlMouseButtonRelease, request, data, len, m_aesDirect)

    unsigned int bt;
#ifdef Q_OS_WIN
    switch ((Qt::MouseButton)request.button())
    {
    case Qt::LeftButton:
        bt = MOUSEEVENTF_LEFTUP;
        break;
    case Qt::RightButton:
        bt = MOUSEEVENTF_RIGHTUP;
        break;
    case Qt::MiddleButton:
        bt = MOUSEEVENTF_MIDDLEUP;
        break;
    default:
        Logger::warning("%s:%d - unknow button %d, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        (int)request.button(), connection()->peerIP().c_str(), connection()->peerPort());
        return;
    }

    SetCursorPos(request.x(), request.y());
    mouse_event(bt, 0, 0, 0, 0);
#else
    switch ((Qt::MouseButton)request.button())
    {
    case Qt::LeftButton:
        bt = LEFT_BUTTON;
        break;
    case Qt::RightButton:
        bt = RIGHT_BUTTON;
        break;
    case Qt::MiddleButton:
        bt = MID_BUTTON;
        break;
    default:
        Logger::warning("%s:%d - unknow button %d, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        (int)request.button(), connection()->peerIP().c_str(), connection()->peerPort());
        return;
    }

    ScreenCapturer::share()->mouseRelease(bt, request.x(), request.y());
#endif

    ScreenCapturer::share()->wakeup();
}

void ScreenGuardAbility::handleCommandOfMouseButtonDoubleClick(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Screen::ControlMouseButtonDoubleClick, request, data, len, m_aesDirect)

#ifdef Q_OS_WIN
    unsigned int bt;
    switch ((Qt::MouseButton)request.button())
    {
    case Qt::LeftButton:
        bt = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
        break;
    case Qt::RightButton:
        bt =  MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP;
        break;
    case Qt::MiddleButton:
        bt =  MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP;
        break;
    default:
        Logger::warning("%s:%d - unknow button %d, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        (int)request.button(), connection()->peerIP().c_str(), connection()->peerPort());
        return;
    }

    SetCursorPos(request.x(), request.y());
    mouse_event(bt, 0, 0, 0, 0);
    mouse_event(bt, 0, 0, 0, 0);
#else
    unsigned int bt;
    switch ((Qt::MouseButton)request.button())
    {
    case Qt::LeftButton:
        bt = LEFT_BUTTON;
        break;
    case Qt::RightButton:
        bt = RIGHT_BUTTON;
        break;
    case Qt::MiddleButton:
        bt = MID_BUTTON;
        break;
    default:
        Logger::warning("%s:%d - unknow button %d, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__,
                        (int)request.button(), connection()->peerIP().c_str(), connection()->peerPort());
        return;
    }

    ScreenCapturer::share()->mouseDoubleClick(bt, request.x(), request.y());
#endif

    ScreenCapturer::share()->wakeup();
}

void ScreenGuardAbility::handleCommandOfMouseWheel(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Screen::ControlMouseWheel, request, data, len, m_aesDirect)

#ifdef Q_OS_WIN
    mouse_event(MOUSEEVENTF_WHEEL, 0, 0, request.delta(), 0);
#else
    ScreenCapturer::share()->mouseWheel(request.delta());
#endif

    ScreenCapturer::share()->wakeup();
}

void ScreenGuardAbility::handleCommandOfSetClipboardText(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Screen::SetClipboardText, request, data, len, m_aesDirect)

    Clipboard::share()->setText(request.text());
}

void ScreenGuardAbility::handleCommandOfSetAutoPublishDesktop(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Screen::EnableAutoPublishDesktop, request, data, len, aes())

    if(request.enable())
        ScreenCapturer::share()->start();
    else
        ScreenCapturer::share()->stop();
}

void ScreenGuardAbility::handleCommandOfShortcut(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::Screen::Shortcut, request, data, len, m_aesDirect)

    switch ((Shortcut)request.key())
    {
    case Shortcut::SHORTCUT_CTRL_SHIFT_ESC:
        pressKey(Qt::Key_Control);
        pressKey(Qt::Key_Shift);
        pressKey(Qt::Key_Escape);
        releaseKey(Qt::Key_Control);
        releaseKey(Qt::Key_Shift);
        releaseKey(Qt::Key_Escape);
        break;
    case Shortcut::SHORTCUT_PRINT_SCREEN:
        pressKey(Qt::Key_Print);
        releaseKey(Qt::Key_Print);
        break;
    }
}

void ScreenGuardAbility::publishScreenFrame(User::ID userId, int64_t seq,  unsigned w, unsigned h, const unsigned char* data, size_t len)
{
    if (!isLoggedIn())
        return;

    if (!len)
        return;

    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    EasyIO::ByteBuffer buf;
    Global::Protocol::Screen::PublishScreenFrame request;
    request.set_seq(seq);
    request.set_w(w);
    request.set_h(h);
    request.set_data(data, len);

    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_PUBLISH_SCREEN_FRAME, request, buf, m_aesDirect)
    connection()->send(buf);
}

void ScreenGuardAbility::publishClipboardText(const std::string &text)
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    if (!m_aesDirect)
        return;

    Global::Protocol::Screen::SetClipboardText request;
    request.set_text(text);

    EasyIO::ByteBuffer buf;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_SET_CLIPBOARD_TEXT, request, buf, m_aesDirect)
    connection()->send(buf);
}

void ScreenGuardAbility::handleLoggedinEvent()
{

}

void ScreenGuardAbility::handleConnectionLostEvent()
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    m_subscribers.clear();
}

void ScreenGuardAbility::pressKey(Qt::Key key)
{
    int k;
    auto it = KEY_MAP.find(key);
    if (it == KEY_MAP.end())
        return;
    k = it->second;

#ifdef Q_OS_WIN
    keybd_event(k, 0, 0, 0);
#else
    ScreenCapturer::share()->keyPress(k);
#endif
}

void ScreenGuardAbility::releaseKey(Qt::Key key)
{
    int k;
    auto it = KEY_MAP.find(key);
    if (it == KEY_MAP.end())
        return;
    k = it->second;

#ifdef Q_OS_WIN
    keybd_event(k, 0, KEYEVENTF_KEYUP, 0);
#else
    ScreenCapturer::share()->keyRelease(k);
#endif
}
