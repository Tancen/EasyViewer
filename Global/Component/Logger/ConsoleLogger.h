#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include "ILogger.h"

class ConsoleLogger : public ILogger
{
public:
    ConsoleLogger();

    void info(const char* log, std::va_list va);
    void debug(const char* log, std::va_list va);
    void warning(const char* log, std::va_list va);
    void error(const char* log, std::va_list va);
    void remove(unsigned year, unsigned month, unsigned day, int hour);
    void setLevel(Level level);

private:
    Level m_level;
};

#endif // CONSOLELOGGER_H
