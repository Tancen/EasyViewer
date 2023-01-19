#ifndef AUTHCHECKER_H
#define AUTHCHECKER_H

#include <string>
#include <mutex>

class AuthChecker
{
public:
    static AuthChecker* share();
    static void init();

    std::string getAuthString();
    void setAuthString(const std::string& str);
    bool test(const std::string& str);

private:
    AuthChecker();

private:
    std::string m_authString;
    std::mutex m_lock;

    static AuthChecker s_this;
};

#endif // AUTHCHECKER_H
