#ifndef EASYIO_H
#define EASYIO_H

#include "EasyIOTCPIConnection.h"
#include "EasyIOTCPIClient.h"
#include "EasyIOTCPIServer.h"
#include "EasyIOUDPISocket.h"
#include "EasyIOUDPIClient.h"
#include "EasyIOUDPIServer.h"
#include "EasyIOEventLoopGroup.h"

#ifdef __linux__
    #include "EasyIOTCPClient_linux.h"
    #include "EasyIOTCPServer_linux.h"
    #include "EasyIOEventLoop_linux.h"
#else
    #include "EasyIOTCPClient_win.h"
    #include "EasyIOTCPServer_win.h"
    #include "EasyIOEventLoop_win.h"
#endif

#include "EasyIOUDPClient.h"
#include "EasyIOUDPServer.h"

#endif
