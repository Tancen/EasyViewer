#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>

struct Account
{
    int64_t userId;
    std::string account;
    std::string password;
    bool disabled;
};

struct BlockedAccount
{
    int64_t userId;
    std::string account;
    long long until;
};

struct BlockedIPAddress
{
    std::string ip;
    long long until;
};

#endif // STRUCTS_H
