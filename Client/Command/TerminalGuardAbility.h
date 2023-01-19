#ifndef TERMINALGUARDABILITY_H
#define TERMINALGUARDABILITY_H

#include "Guard.h"
#include "Command/TerminalManager.h"

class TerminalGuardAbility : public Guard::IAbility
{
public:
    TerminalGuardAbility();
    ~TerminalGuardAbility();

    int role() override;

    bool handleCompleteData(unsigned tag, const unsigned char* data, size_t len ) override;


private:
    void handleCommandOfCreateTerminal(const unsigned char* data, size_t len);
    void handleCommandOfWriteCommand(const unsigned char* data, size_t len);
    void handleCommandOfCloseTerminal(const unsigned char* data, size_t len);
    void handleCommandOfResizeTerminal(const unsigned char* data, size_t len);

    void publishTerminalOutput(User::ID userId, const std::string& terminalId, const char* src, size_t len);
    void kickoutVisitor(User::ID userId, const std::string& terminalId);

    void handleLoggedinEvent() override;
    void handleConnectionLostEvent() override;

private:
    TerminalManager* m_terminalManager;
    std::recursive_mutex m_mutex;
};

#endif // TERMINALGUARDABILITY_H
