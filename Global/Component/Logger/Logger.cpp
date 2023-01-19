#include "Logger.h"
#include <assert.h>

std::unique_ptr<ILogger> Logger::s_logger;

#define CALL(_func, _log) \
    va_list ap;   \
    va_start(ap, _log);\
    s_logger->_func(_log, ap); \
    va_end(ap);


bool Logger::init(std::unique_ptr<ILogger> logger)
{
    assert(!s_logger.get());
    s_logger = std::move(logger);
    return true;
}

void Logger::info(const char *log, ...)
{
    CALL(info, log)
}

void Logger::debug(const char *log, ...)
{
    CALL(debug, log)
}

void Logger::warning(const char *log, ...)
{
    CALL(warning, log)
}

void Logger::error(const char *log, ...)
{
    CALL(error, log)
}

void Logger::remove(unsigned year, unsigned month, unsigned day, int hour)
{
    s_logger->remove(year, month, day, hour);
}

void Logger::setLevel(ILogger::Level level)
{
    s_logger->setLevel(level);
}
