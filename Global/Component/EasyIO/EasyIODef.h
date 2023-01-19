#ifndef EASYIODEF_H
#define EASYIODEF_H

#ifdef __linux__
    #include <unistd.h>

    typedef int SOCKET;

    #define INVALID_SOCKET	(SOCKET)(~0)
    #define closesocket(fd)  ::close((fd))
#else
    #include <winsock2.h>
    typedef int socklen_t;
#endif

#endif
