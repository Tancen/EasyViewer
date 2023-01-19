#include "FileLogger.h"
#include <mutex>
#include <stdarg.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <qdebug.h>

#define INDEX_INFO      0
#define INDEX_DEBUG     1
#define INDEX_WARNING   2
#define INDEX_ERROR     3
#define MAX_INDEX       4


#define CALL_PUSH(_prefix, _log, _va)  \
{\
    std::va_list _dupva; \
    QDateTime _time = QDateTime::currentDateTime();\
    std::string _prefixedLog = _time.toString("yyyy-MM-dd hh:mm:ss.zzz").toStdString();\
    _prefixedLog.append(" ").append(_prefix).append(" - ").append(_log);\
    \
    {\
        va_copy(_dupva, _va); \
        qDebug() << QString().vasprintf(_prefixedLog.c_str(), _dupva);\
        va_end(_dupva);\
    }\
    _prefixedLog.append("\n");\
    {\
        va_copy(_dupva, _va); \
        push(_time, _prefixedLog, _dupva);\
        va_end(_dupva);\
    }\
}


const char** FileLogger::PREFIX = []()
{
    static const char* _PREFIX[MAX_INDEX] = {
        "INFO",
        "DEBUG",
        "WARNING",
        "ERROR"
    };

    return _PREFIX;
}();

void FileLogger::info(const char* log, std::va_list va)
{
    if (m_level > L_INFO)
        return;

    CALL_PUSH(PREFIX[INDEX_INFO], log, va);
}

void FileLogger::debug(const char* log, std::va_list va)
{
    if (m_level > L_DEBUG)
        return;

    CALL_PUSH(PREFIX[INDEX_DEBUG], log, va);
}

void FileLogger::warning(const char* log, std::va_list va)
{
    if (m_level > L_WARNING)
        return;

    CALL_PUSH(PREFIX[INDEX_WARNING], log, va);
}

void FileLogger::error(const char* log, std::va_list va)
{
    if (m_level > L_ERROR)
        return;

    CALL_PUSH(PREFIX[INDEX_ERROR], log, va);
}

void FileLogger::remove(unsigned year, unsigned month, unsigned day, int hour)
{
    char buf[16];
    auto funcRemove = [this, &buf](unsigned year, unsigned month, unsigned day, int hour)
    {
        sprintf(buf, "%04d%02d%02d%02d-", year, month, day, hour);
        std::string fileName = m_path;
        fileName.append(buf)
            .append(m_basename).append(m_suffix);
        QFile::remove(fileName.c_str());

        for (int i = 0; i < MAX_INDEX; ++i)
        {
            sprintf(buf, "%04d%02d%02d%02d-", year, month, day, hour);
            fileName = m_path;
            fileName.append(PREFIX[i]).append("-").append(buf)
                .append(m_basename).append(m_suffix);
            QFile::remove(fileName.c_str());
        }
    };

    if (hour >= 0)
    {
        funcRemove(year, month, day, hour);
    }
    else
    {
        for (int i = 0; i < 24; ++i)
            funcRemove(year, month, day, i);
    }
}

void FileLogger::setLevel(FileLogger::Level level)
{
    m_level = level;
}

FileLogger::FileLogger(const std::string &filePath)
    :   m_level(L_DEBUG)
{
    QFileInfo info(filePath.c_str());
    m_path = info.absolutePath().toStdString() + + "/";
    m_basename = info.completeBaseName().toStdString();
    m_suffix = QString(".").append(info.completeSuffix()).toStdString();
}

FileLogger::~FileLogger()
{

}

void FileLogger::push(const QDateTime &time, const std::string &log, std::va_list args)
{
    std::string fileName = m_path;
    fileName.append(time.toString("yyyyMMddhh-").toStdString())
        .append(m_basename).append(m_suffix);

    write(fileName, log, args);
}

void FileLogger::write(const std::string &file, const std::string &log, std::va_list args)
{
#ifdef Q_OS_WIN
    m_mutex.lock();
#endif

    FILE *pf = NULL;

#ifdef Q_OS_WIN
    pf = _wfopen(QString(file.c_str()).toStdWString().c_str(), L"a");
#else
    pf = fopen(file.c_str(), "a");
#endif

    if (!pf)
    {
#ifdef Q_OS_WIN
        m_mutex.unlock();
#endif
        return;
    }

    vfprintf(pf, log.c_str(), args);
    fclose(pf);

#ifdef Q_OS_WIN
    m_mutex.unlock();
#endif
}

void FileLogger::push(const QDateTime& time, const std::string& prefix,
    const std::string &log, std::va_list args)
{
    std::string fileName;
    fileName.append(time.toString("yyyyMMddhh-").toStdString())
        .append(m_basename).append(m_suffix);

    fileName = prefix + "-" + fileName;
    write(m_path + fileName, log, args);
}
