#ifndef ILOGGER_H
#define ILOGGER_H

#include <cstdarg>

class ILogger
{
public:
    enum Level {  L_DEBUG, L_INFO, L_WARNING, L_ERROR };

public:
    ILogger(){}
    virtual ~ILogger(){}

    virtual void info(const char* log, std::va_list va) = 0;
    virtual void debug(const char* log, std::va_list va) = 0;
    virtual void warning(const char* log, std::va_list va) = 0;
    virtual void error(const char* log, std::va_list va) = 0;
    virtual void remove(unsigned year, unsigned month, unsigned day, int hour) = 0;
    virtual void setLevel(Level level) = 0;
};

#endif // ILOGGER_H
