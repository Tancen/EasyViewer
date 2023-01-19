#ifndef FILELOGGER_H
#define FILELOGGER_H

#include "ILogger.h"

#include <string>
#include <qdatetime.h>
#include <mutex>
#include <cstdarg>


class FileLogger : public ILogger
{
public:
    FileLogger(const std::string &filePath);
    ~FileLogger();

    void info(const char* log, std::va_list va);
    void debug(const char* log, std::va_list va);
    void warning(const char* log, std::va_list va);
    void error(const char* log, std::va_list va);
    void remove(unsigned year, unsigned month, unsigned day, int hour);
    void setLevel(Level level);

private:
    void push(const QDateTime& time, const std::string& prefix,
            const std::string &log, std::va_list args);
    void push(const QDateTime& time, const std::string &log, std::va_list args);
    void write(const std::string &file, const std::string &log, std::va_list args);

    void prefixLog(const QDateTime& time, const std::string& prefix,
                   const std::string &log, std::string& dst);

private:
    std::string m_basename;
    std::string m_path;
    std::string m_suffix;

    Level m_level;
    static const char** PREFIX;

#ifdef Q_OS_WIN
    std::recursive_mutex m_mutex;
#endif

};

#endif // FILELOGGER_H
