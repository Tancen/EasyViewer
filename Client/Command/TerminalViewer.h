#ifndef TERMINALVIEWER_H
#define TERMINALVIEWER_H

#include "ViewerBase.h"

class TerminalViewer : public ViewerBase
{
public:
    TerminalViewer(const std::string& peerHost, const std::string& peerIP, unsigned short peerPort,  const std::string& publicKey,
                   const std::string& account, const std::string& password,
                   User::ID partnerUserId, const std::string& authString);
    ~TerminalViewer();

    int role() override;
    std::pair<bool, std::string> init() override;

private:
    bool handleCompleteData(unsigned tag, const unsigned char* data, size_t len ) override;

    void handleResponseOfCreateTerminal(const unsigned char* data, size_t len);
    void handlePublishTerminalOutput(const unsigned char* data, size_t len);

    void whenConnectFailed(EasyIO::TCP::IConnection* con, const std::string& reason) override;
    void whenDisconnected(EasyIO::TCP::IConnection* con) override;
    void whenLoggedIn() override;
    void whenLoginFailed(int errCode, const std::string& errString) override;

    void writeCharactersToPartner(const char* hits, size_t len);
    void resizePartnerTerminal();

    void error(int errCode, const std::string& errString);

    bool getConsoleSize(short& w, short& h);

private:
    bool m_disableInput;
    std::unique_ptr<Cryptology::AES<EasyIO::ByteBuffer>> m_aesDirect;
};

#endif // TERMINALVIEWER_H
