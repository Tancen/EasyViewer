#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include "Global/Define.h"

struct Account
{
    User::ID id = 0;
    std::string account;
    std::string password;
    bool disabled = false;
};

#endif // ACCOUNT_H
