#ifndef ICONNECTION_H
#define ICONNECTION_H

#include <memory>
#include <string>

class IConnection
{
public:
    virtual const std::string& peerIP() = 0;
    virtual unsigned short peerPort() = 0;
    virtual void disconnect() = 0;
    virtual void send(const unsigned char* data, size_t len) = 0;

};

typedef std::shared_ptr<IConnection> IConnectionPtr;

#endif // ICONNECTION_H
