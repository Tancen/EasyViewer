#include "TerminalViewer.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Terminal/Terminal.pb.h"
#include "Global/Protocol/Error.h"
#include "Global/Define.h"
#include "KeyboardHit.h"
#include "Global/Component/Logger/Logger.h"
#include "UsualParseProtobufDataMacro.h"
#include <QCoreApplication>
#ifdef Q_OS_WIN
    #include <windows.h>
#else
    #include <sys/ioctl.h>
#endif

TerminalViewer::TerminalViewer(
        const std::string& peerHost, const std::string& peerIP, unsigned short peerPort,
        const std::string& publicKey,
        const std::string &account, const std::string &password, User::ID partnerUserId,
        const std::string &authString)
    :   ViewerBase(peerHost, peerIP, peerPort, publicKey, account, password, partnerUserId, authString),
        m_disableInput(true)
{
    KeyboardHit::share()->subscribe(this,
                            std::bind(&TerminalViewer::writeCharactersToPartner, this, std::placeholders::_1, std::placeholders::_2),
                            std::bind(&TerminalViewer::resizePartnerTerminal, this));
}

TerminalViewer::~TerminalViewer()
{
    KeyboardHit::share()->unsubscribe(this);
}

int TerminalViewer::role()
{
    return GLOBAL_CONNECTION_ROLE_TERMINAL_VISITOR;
}

void TerminalViewer::whenLoggedIn()
{
    short w = 0, h = 0;
    if (!getConsoleSize(w, h))
    {
        int err;
#ifdef Q_OS_WIN
        err = GetLastError();
#else
        err = errno;
#endif
        error(GLOBAL_PROTOCOL_ERR_OPERATE_FAILED, "call getConsoleSize failed, err " + std::to_string(err));
        return;
    }

    Global::Protocol::Terminal::RequestCreateTerminal request;
    request.set_user_id(getPartnerUserId());
    request.set_auth_string(getAuthString());
    request.set_width(w);
    request.set_height(h);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_CREATE_TERMINAL, request, data, m_aes)
            send(data);
}

void TerminalViewer::whenLoginFailed(int errCode, const std::string &errString)
{
    error(errCode, errString.c_str());
}

bool TerminalViewer::handleCompleteData(unsigned tag, const unsigned char *data, size_t len)
{
    switch (tag)
    {
    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_CREATE_TERMINAL:
        handleResponseOfCreateTerminal(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_PUBLISH_TERMINAL_OUTPUT:
        handlePublishTerminalOutput(data, len);
        return true;
    }
    return false;
}

void TerminalViewer::handleResponseOfCreateTerminal(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::Terminal::ResponseCreateTerminal, response, data, len, m_aes)

    if (response.err_code() != GLOBAL_PROTOCOL_ERR_NO_ERROR)
    {
        error(response.err_code(), response.err_string());
        qApp->exit(0);
    }
    else
    {
        m_disableInput = false;
        std::string errorString;
        m_aesDirect = Cryptology::AES<EasyIO::ByteBuffer>::newInstance(response.secret_key(), &errorString);
        if (!m_aes)
        {
            error(-1, "init AES instance failed: " + errorString);
            qApp->exit(0);
            return;
        }
        fflush(stdout);

        resizePartnerTerminal();
    }
}

void TerminalViewer::handlePublishTerminalOutput(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::Terminal::PublishTerminalOutput, publish, data, len, m_aesDirect)

    std::string str(publish.data().data(), publish.data().length());
    printf("%s", publish.data().c_str());
    fflush(stdout);
}

void TerminalViewer::whenConnectFailed(EasyIO::TCP::IConnection *con, const std::string &reason)
{
    error(GLOBAL_PROTOCOL_ERR_CONNECT_FAILED, reason.c_str());
}

void TerminalViewer::whenDisconnected(EasyIO::TCP::IConnection *con)
{
    error(GLOBAL_PROTOCOL_ERR_DISCONNECTED, "Disconnected");
}

void TerminalViewer::writeCharactersToPartner(const char* hits, size_t len)
{
    if (m_disableInput)
        return;

    Global::Protocol::Terminal::WriteCommand request;
    request.set_command(hits, len);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_WRITE_COMMAND_TO_TERMINAL, request, data, m_aes)
            send(data);
}

void TerminalViewer::resizePartnerTerminal()
{
    short w = 0, h = 0;
    if (!getConsoleSize(w, h))
    {
        int err;
#ifdef Q_OS_WIN
        err = GetLastError();
#else
        err = errno;
#endif
        error(GLOBAL_PROTOCOL_ERR_OPERATE_FAILED, "call getConsoleSize failed, err " + std::to_string(err));
        return;
    }

    Global::Protocol::Terminal::ResizeTerminal request;
    request.set_width(w);
    request.set_height(h);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESIZE_TERMINAL, request, data, m_aes)
    send(data);
}

void TerminalViewer::error(int errCode, const std::string &errString)
{
    Logger::error("err[%d]: %s", errCode, errString.c_str());
    qApp->exit(errCode);
}

bool TerminalViewer::getConsoleSize(short &w, short &h)
{
#ifdef Q_OS_WIN
    CONSOLE_SCREEN_BUFFER_INFOEX info = { 0 };
    info.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
    if (!GetConsoleScreenBufferInfoEx(GetStdHandle(STD_OUTPUT_HANDLE), &info))
        return false;

    w = info.srWindow.Right - info.srWindow.Left + 1;
    h = info.srWindow.Bottom - info.srWindow.Top + 1;
#else
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    w = size.ws_col;
    h = size.ws_row;
#endif

    return true;
}
