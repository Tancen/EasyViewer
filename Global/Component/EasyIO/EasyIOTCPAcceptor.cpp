#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif
#include "EasyIOTCPAcceptor.h"

using namespace EasyIO::TCP;

Acceptor::Acceptor()
    : m_socket(INVALID_SOCKET),
      m_exit(false),
      onAccepted(nullptr)
{

}

Acceptor::~Acceptor()
{
    m_exit = true;
#ifdef __linux__
    shutdown(m_socket, SHUT_RDWR);
#endif
    closesocket(m_socket);

    if (m_thread.joinable())
        m_thread.join();
}

bool Acceptor::accept(unsigned short port, int backlog, int& err)
{
    if (m_socket != INVALID_SOCKET)
            return false;

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET)
    {
        #ifdef __linux__
            err = errno;
        #else
            err = GetLastError();
        #endif
        return false;
    }

    do
    {
		int v = 1;
		setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&v, sizeof(v));
		
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
	
        if(bind(m_socket, (sockaddr*)&addr, sizeof(addr)) == -1)
            break;

        if(listen(m_socket, backlog) == -1)
            break;

        m_thread = std::thread(std::bind(&Acceptor::execute, this));

        return true;
    }
    while(0);

    #ifdef __linux__
        err = errno;
    #else
        err = GetLastError();
    #endif
    closesocket(m_socket);
    m_socket = INVALID_SOCKET;
    return false;
}

void Acceptor::execute()
{
    socklen_t lenAddr = sizeof(sockaddr_in);
    sockaddr_in addr;
    SOCKET sock;

    while (!m_exit)
    {
        sock = ::accept(m_socket, (sockaddr*)&addr, &lenAddr);
#ifdef __linux__
        if (sock > 0)
#else
        if (sock != INVALID_SOCKET)
#endif
        {
            if (onAccepted)
                onAccepted(sock);
            else
                closesocket(sock);
        }
    }
}
