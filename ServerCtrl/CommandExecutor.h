#ifndef COMMANDEXECUTOR_H
#define COMMANDEXECUTOR_H

#include "Client.h"
#include <iterator>

class CommandExecutor
{
public:
    CommandExecutor();

    void execute(const std::string& command, Client& client);

private:
    void execute(std::list<std::string>& args, Client& client);
    void executeAroundUser(std::list<std::string>& args, Client& client);
    void executeAroundBlackList(std::list<std::string>& args, Client& client);

    bool popNextArg(std::list<std::string>& args, std::string& dst);
};

#endif // COMMANDEXECUTOR_H
