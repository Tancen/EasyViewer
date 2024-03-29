﻿#ifndef GLOBAL_DEFINE_H
#define GLOBAL_DEFINE_H

#include <QtGlobal>

#define GLOBAL_CONNECTION_ROLE_SCREEN               1
#define GLOBAL_CONNECTION_ROLE_FILE_TRANSMISSION    2
#define GLOBAL_CONNECTION_ROLE_TERMINAL             4

#define GLOBAL_CONNECTION_ROLE_FLAG_GUARD                   0x10000
#define GLOBAL_CONNECTION_ROLE_SCREEN_GUARD                 (GLOBAL_CONNECTION_ROLE_FLAG_GUARD | GLOBAL_CONNECTION_ROLE_SCREEN)
#define GLOBAL_CONNECTION_ROLE_FILE_TRANSMISSION_GUARD      (GLOBAL_CONNECTION_ROLE_FLAG_GUARD | GLOBAL_CONNECTION_ROLE_FILE_TRANSMISSION)
#define GLOBAL_CONNECTION_ROLE_TERMINAL_GUARD               (GLOBAL_CONNECTION_ROLE_FLAG_GUARD | GLOBAL_CONNECTION_ROLE_TERMINAL)

#define GLOBAL_CONNECTION_ROLE_FLAG_VISITOR                 0x20000
#define GLOBAL_CONNECTION_ROLE_SCREEN_VISITOR               (GLOBAL_CONNECTION_ROLE_FLAG_VISITOR | GLOBAL_CONNECTION_ROLE_SCREEN)
#define GLOBAL_CONNECTION_ROLE_FILE_TRANSMISSION_VISITOR    (GLOBAL_CONNECTION_ROLE_FLAG_VISITOR | GLOBAL_CONNECTION_ROLE_FILE_TRANSMISSION)
#define GLOBAL_CONNECTION_ROLE_TERMINAL_VISITOR             (GLOBAL_CONNECTION_ROLE_FLAG_VISITOR | GLOBAL_CONNECTION_ROLE_TERMINAL)

#define GLOBAL_CONNECTION_ROLE_FLAG_MANAGER                 0x40000
#define GLOBAL_CONNECTION_ROLE_MANAGER                      (GLOBAL_CONNECTION_ROLE_FLAG_MANAGER | 0)

#ifdef Q_OS_WIN
    #ifndef __PRETTY_FUNCTION__
        #define __PRETTY_FUNCTION__  __FUNCSIG__
    #endif
#endif

struct User
{
    typedef int64_t ID;
};

#endif // GLOBAL_DEFINE_H
