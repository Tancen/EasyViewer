#ifndef LOGGER_H
#define LOGGER_H

#include "ILogger.h"
#include <memory>

class Logger
{
public:

    static bool init(std::unique_ptr<ILogger> logger);

    static void info(const char* log, ...);
    static void debug(const char* log, ...);
    static void warning(const char* log, ...);
    static void error(const char* log, ...);
    static void remove(unsigned year, unsigned month, unsigned day,
        int hour = -1);
    static void setLevel(ILogger::Level level);

private:
    Logger();
    ~Logger();

private:
    static std::unique_ptr<ILogger> s_logger;
};

#endif // LOGGER_H
