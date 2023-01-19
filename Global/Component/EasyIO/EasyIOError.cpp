#include "EasyIOError.h"
#include <string.h>

#if  defined(WIN32) || defined(WIN64)
    #include <Windows.h>
    #include <WinSock2.h>
#else
    #include <sys/socket.h>
#endif

std::string EasyIO::Error::formatError(int err)
{
#if  defined(WIN32) || defined(WIN64)
    char* buf = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, err, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPSTR)&buf, 0, NULL);
    std::string ret(buf, size);
    LocalFree(buf);

    return ret;
#else
    return strerror(err);
#endif
}

int EasyIO::Error::getSocketError(SOCKET socket)
{
    int err = 0;
#if  defined(WIN32) || defined(WIN64)
    int l = sizeof(err);
    if (getsockopt(socket, SOL_SOCKET, SO_ERROR, (char*)&err, &l))
    {
        err = GetLastError();
    }
#else
     socklen_t l = sizeof(err);
     if (getsockopt(socket, SOL_SOCKET, SO_ERROR, &err, &l))
     {
         err = errno;
     }
#endif

    return err;
}
