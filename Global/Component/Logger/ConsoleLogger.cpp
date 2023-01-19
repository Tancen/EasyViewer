#include "ConsoleLogger.h"
#include <QDateTime>
#include <stdio.h>
#include <QtDebug>

#define PRINT(_fd, _prefix, _log, _va)  \
{\
    fprintf(_fd, "\n"); \
    QDateTime _time = QDateTime::currentDateTime();\
    std::string _prefixedLog = _time.toString("yyyy-MM-dd hh:mm:ss.zzz").toStdString();\
    _prefixedLog.append(" ").append(_prefix).append(" - ").append(_log);\
    vfprintf(_fd, _prefixedLog.c_str(), _va);\
}

ConsoleLogger::ConsoleLogger()
    :   m_level(L_DEBUG)
{

}

void ConsoleLogger::info(const char *log, std::va_list va)
{
    if (m_level > L_INFO)
        return;

    PRINT(stdout, "INFO", log, va);
}

void ConsoleLogger::debug(const char *log, std::va_list va)
{
    if (m_level > L_DEBUG)
        return;

    PRINT(stdout, "DEBUG", log, va);
}

void ConsoleLogger::warning(const char *log, std::va_list va)
{
    if (m_level > L_DEBUG)
        return;

    PRINT(stdout, "WARNING", log, va);
}

void ConsoleLogger::error(const char *log, std::va_list va)
{
    if (m_level > L_ERROR)
        return;

    PRINT(stderr, "ERROR", log, va);
}

void ConsoleLogger::remove(unsigned year, unsigned month, unsigned day, int hour)
{

}

void ConsoleLogger::setLevel(ILogger::Level level)
{
    m_level = level;
}
