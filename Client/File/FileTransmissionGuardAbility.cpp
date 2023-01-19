#include "FileTransmissionGuardAbility.h"
#include <QtGlobal>
#include "Global/Define.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/File/File.pb.h"
#include "Global/Protocol/Error.h"
#include "AuthChecker.h"
#include "FileTransmission.h"
#include <cinttypes>
#include <zlib.h>
#include <zconf.h>
#include <QDateTime>
#include "UsualParseProtobufDataMacro.h"

#ifdef Q_OS_WIN
    #include <shlobj_core.h>
#endif

#define PROTOBUF_FILL_ENTRIES(_dir, _dst) \
{ \
    QVector<FileTransmission::EntryInfo> _entries; \
    _entries.reserve(4096); \
    FileTransmission::getEntries(_dir, _entries); \
    for (const auto& e : _entries) \
    { \
        auto entry = _dst.add_entries(); \
        entry->set_name(e.name); \
        entry->set_directory(e.directory); \
        entry->set_size(e.type == FileTransmission::EntryType::DIRECTORY ? 0 : e.size); \
        entry->set_is_directory(e.type == FileTransmission::EntryType::DIRECTORY ? true : false); \
        entry->set_last_modifying_time(e.lastModifyingTime.toMSecsSinceEpoch()); \
    } \
}

using namespace std::placeholders;

FileTransmissionGuardAbility::FileTransmissionGuardAbility()
{
    FileTransmission::TaskScheduler::share()->subscribeEvents(this,
                std::bind(&FileTransmissionGuardAbility::whenTaskAdded, this, _1),
                std::bind(&FileTransmissionGuardAbility::whenTaskRemoved, this, _1),
                std::bind(&FileTransmissionGuardAbility::whenTaskRemoteOpenFile, this, _1),
                std::bind(&FileTransmissionGuardAbility::whenTaskRemoteCloseFile, this, _1),
                std::bind(&FileTransmissionGuardAbility::whenTaskStateChanged, this, _1),
                std::bind(&FileTransmissionGuardAbility::whenTaskProgressChanged, this, _1),
                std::bind(&FileTransmissionGuardAbility::whenTaskRemoteWriteFileBlock, this, _1, _2, _3, _4),
                std::bind(&FileTransmissionGuardAbility::whenTaskRemoteReadFileBlock, this, _1, _2, _3)
            );
}

FileTransmissionGuardAbility::~FileTransmissionGuardAbility()
{
    FileTransmission::TaskScheduler::share()->unsubscribeEvents(this);
}

int FileTransmissionGuardAbility::role()
{
    return GLOBAL_CONNECTION_ROLE_FILE_TRANSMISSION;
}

bool FileTransmissionGuardAbility::handleCompleteData(unsigned tag, const unsigned char *data, size_t len)
{
    switch (tag)
    {
    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISIT_FILES2:
        handleCommandOfVisitFiles(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_MAKE_DIRECTORY :
        handleCommandOfMakeDirectory(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_DELETE_ENTRY :
        handleCommandOfDeleteEntry(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_RENAME_ENTRY :
        handleCommandOfRenameEntry(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISITING_PATH_GO_UP :
        handleCommandOfGoUp(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISITING_PATH_GO_HOME :
        handleCommandOfGoHome(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_ENTRIES :
        handleCommandOfListEntries(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_OPEN_FILE :
        handleCommandOfOpenFile(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CLOSE_FILE :
        handleCommandOfCloseFile(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_READ_FILE_BLOCK :
        handleCommandOfReadFile(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_WRITE_FILE_BLOCK :
        handleCommandOfWriteFile(data, len);
        return true;
    }

    return false;
}

void FileTransmissionGuardAbility::handleCommandOfVisitFiles(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::File::RequestVisitFiles2, request, data, len, aes())

    std::string errorString;
    int err = GLOBAL_PROTOCOL_ERR_NO_ERROR;
    EasyIO::ByteBuffer buf;
    Global::Protocol::File::ResponseVisitFiles2 response;
    response.set_async_task_id(request.async_task_id());
    response.set_async_task_certificate(request.async_task_certificate());

    do
    {
        if(!AuthChecker::share()->test(request.auth_string()))
        {
            err = GLOBAL_PROTOCOL_ERR_PASSWORD_INCORRECT;
            errorString = Global::Protocol::formatError(err);
            break;
        }

        QDir dir;
        if (!dir.cd(dir.homePath()))
        {
            err = GLOBAL_PROTOCOL_ERR_PATH_UNREACHABLE;
            errorString = Global::Protocol::formatError(err);
            break;
        }

        #ifdef Q_OS_WIN
            char path[256] = {0};
            if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, path)))
            {
                std::replace(path + 0, path + (sizeof(path) - 1), '\\', '/');
                response.add_default_dirs(path);
            }
            DWORD m = GetLogicalDrives();
            if (m)
            {
                for (int i = 0; i < sizeof(m) * 8 && i < 26; i++)
                {
                    if (m & (1 << i))
                    {
                        response.add_default_dirs(std::string(1, 'A' + i).append(":/"));
                    }
                }
            }
            response.add_default_dirs(dir.path().toStdString());
        #else
            response.add_default_dirs(dir.path().toStdString());
            response.add_default_dirs("/");
        #endif

        if (response.default_dirs_size())
        {
            dir.cd(response.default_dirs(0).c_str());
            PROTOBUF_FILL_ENTRIES(dir, response)
        }
    } while(0);


    response.set_err_code(err);
    response.set_err_string(errorString);
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISIT_FILES2, response, buf, aes())
    connection()->send(buf);
}

void FileTransmissionGuardAbility::handleCommandOfGoUp(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::File::RequestGoUp, request, data, len, aes())

    int err = GLOBAL_PROTOCOL_ERR_NO_ERROR;
    EasyIO::ByteBuffer buf;
    Global::Protocol::File::ResponseGoUp response;
    response.set_seq(request.seq());
    response.set_user_id(request.user_id());

    QDir dir;
    if(!dir.cd(request.dir().c_str()) || !dir.cdUp())
    {
        err = GLOBAL_PROTOCOL_ERR_PATH_UNREACHABLE;
    }
    else
    {
        response.set_dir(dir.path().toStdString());
        PROTOBUF_FILL_ENTRIES(dir, response)
    }

    response.set_err_code(err);
    response.set_err_string(Global::Protocol::formatError(err));
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISITING_PATH_GO_UP, response, buf, aes())
    connection()->send(buf);
}

void FileTransmissionGuardAbility::handleCommandOfGoHome(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::File::RequestGoHome, request, data, len, aes())

    int err = GLOBAL_PROTOCOL_ERR_NO_ERROR;
    EasyIO::ByteBuffer buf;
    Global::Protocol::File::ResponseGoHome response;
    response.set_seq(request.seq());
    response.set_user_id(request.user_id());

    QDir dir;
    if(!dir.cd(dir.homePath()))
    {
        err = GLOBAL_PROTOCOL_ERR_PATH_UNREACHABLE;
    }
    else
    {
        response.set_dir(dir.path().toStdString());
        PROTOBUF_FILL_ENTRIES(dir, response)
    }

    response.set_err_code(err);
    response.set_err_string(Global::Protocol::formatError(err));
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISITING_PATH_GO_HOME, response, buf, aes())
    connection()->send(buf);
}

void FileTransmissionGuardAbility::handleCommandOfListEntries(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::File::RequestListEntries, request, data, len, aes())

    int err = GLOBAL_PROTOCOL_ERR_NO_ERROR;
    EasyIO::ByteBuffer buf;
    Global::Protocol::File::ResponseListEntries response;
    response.set_seq(request.seq());
    response.set_user_id(request.user_id());

    QDir dir;
    if(!dir.cd(request.dir().c_str()))
    {
        err = GLOBAL_PROTOCOL_ERR_PATH_UNREACHABLE;
    }
    else
    {
        response.set_dir(dir.path().toStdString());
        PROTOBUF_FILL_ENTRIES(dir, response)
    }

    response.set_err_code(err);
    response.set_err_string(Global::Protocol::formatError(err));
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LIST_ENTRIES, response, buf, aes())
    connection()->send(buf);
}

void FileTransmissionGuardAbility::handleCommandOfDeleteEntry(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::File::RequestDeleteEntry, request, data, len, aes())

    int err = GLOBAL_PROTOCOL_ERR_NO_ERROR;
    EasyIO::ByteBuffer buf;
    Global::Protocol::File::ResponseDeleteEntry response;
    response.set_seq(request.seq());
    response.set_user_id(request.user_id());

    do
    {
        std::string path = request.dir() + "/" + request.name();
        QFileInfo  info(path.c_str());
        if (!info.exists())
        {
            err = GLOBAL_PROTOCOL_ERR_PATH_UNREACHABLE;
            break;
        }

        QDir dir;
        if (!dir.cd(request.dir().c_str()))
        {
            err = GLOBAL_PROTOCOL_ERR_PATH_UNREACHABLE;
            break;
        }

        bool b;
        if (info.isDir())
            b = dir.cd(request.name().c_str()) && dir.removeRecursively();
        else
            b = dir.remove(request.name().c_str());

        if (!b)
            err = GLOBAL_PROTOCOL_ERR_OPERATE_FAILED;

        response.set_dir(request.dir());
        response.set_name(request.name());

    } while (0);

    response.set_err_code(err);
    response.set_err_string(Global::Protocol::formatError(err));
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_DELETE_ENTRY, response, buf, aes())
    connection()->send(buf);
}

void FileTransmissionGuardAbility::handleCommandOfMakeDirectory(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::File::RequestMakeDirectory, request, data, len, aes())

    int err = GLOBAL_PROTOCOL_ERR_NO_ERROR;
    EasyIO::ByteBuffer buf;
    Global::Protocol::File::ResponseMakeDirectory response;
    response.set_seq(request.seq());
    response.set_user_id(request.user_id());

    do
    {
        QDateTime now = QDateTime::currentDateTime();
        QDir dir(request.dir().c_str());
        if (!dir.mkdir(request.name().c_str()))
        {
            err = GLOBAL_PROTOCOL_ERR_OPERATE_FAILED;
            break;
        }

        response.set_dir(request.dir());
        response.set_name(request.name());
        response.set_last_modifying_time(now.toString(GLOBAL_PROTOCOL_DATETIME_FORMAT).toStdString());

    } while (0);

    response.set_err_code(err);
    response.set_err_string(Global::Protocol::formatError(err));
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_MAKE_DIRECTORY, response, buf, aes())
    connection()->send(buf);
}

void FileTransmissionGuardAbility::handleCommandOfRenameEntry(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::File::RequestRenameEntry, request, data, len, aes())

    int err = GLOBAL_PROTOCOL_ERR_NO_ERROR;
    EasyIO::ByteBuffer buf;
    Global::Protocol::File::ResponseRenameEntry response;
    response.set_seq(request.seq());
    response.set_user_id(request.user_id());

    do
    {
        QDir dir(request.dir().c_str());
        if (!dir.rename(request.old_name().c_str(), request.new_name().c_str()))
        {
            err = GLOBAL_PROTOCOL_ERR_OPERATE_FAILED;
            break;
        }

        response.set_dir(request.dir());
        response.set_old_name(request.old_name());
        response.set_new_name(request.new_name());
    } while (0);

    response.set_err_code(err);
    response.set_err_string(Global::Protocol::formatError(err));
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_RENAME_ENTRY, response, buf, aes())
    connection()->send(buf);
}

void FileTransmissionGuardAbility::handleCommandOfOpenFile(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::File::RequestOpenFile, request, data, len, aes())

    FileTransmission::TaskScheduler::share()->removeTask(request.task_id());
    auto err = FileTransmission::TaskScheduler::share()->addTaskPassively(request.task_id(), m_entity->host(), m_entity->port(),
            request.user_id(),
            (FileTransmission::Task::Action)request.mode(),
            request.directory(), request.file_name(),
            request.seek(), request.file_size(), request.force());


    EasyIO::ByteBuffer buf;
    Global::Protocol::File::ResponseOpenFile response;
    response.set_seq(request.seq());
    response.set_user_id(request.user_id());
    response.set_task_id(request.task_id());
    response.set_err_code(err.first);
    response.set_err_string(err.second);
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_OPEN_FILE, response, buf, aes())
    connection()->send(buf);
}

void FileTransmissionGuardAbility::handleCommandOfCloseFile(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::File::CloseFile, request, data, len, aes())

    FileTransmission::TaskScheduler::share()->removeTask(request.task_id());
}

void FileTransmissionGuardAbility::handleCommandOfReadFile(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::File::RequestReadFileBlock, request, data, len, aes())

    Global::Protocol::File::ResponseReadFileBlock response;
    std::pair<int, std::string> err;
    do
    {
        size_t l0 = request.len();
        EasyIO::ByteBuffer rawFileData(l0);
        err = FileTransmission::TaskScheduler::share()->readLocalFileBlock(request.task_id(), request.seek(),
                                       rawFileData.readableBytes(), l0);
        if (err.first)
            break;
        rawFileData.moveWriterIndex(l0);

        unsigned long l1 = compressBound(l0);
        EasyIO::ByteBuffer compressedData(l1);
        int zErr = compress((unsigned char*)compressedData.readableBytes(), &l1,
                            (unsigned char*)rawFileData.readableBytes(), rawFileData.numReadableBytes());
        if (zErr != Z_OK)
        {
            Logger::warning("%s:%d - !compress, error %d, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__, zErr,
                            connection()->peerIP().c_str(), connection()->peerPort());
            err.first = GLOBAL_PROTOCOL_ERR_OPERATE_FAILED;
            err.second = "compress data failed";
            break;
        }

        response.set_seek(request.seek());
        response.set_raw_data_len(l0);
        response.set_compressed_data(compressedData.readableBytes(), l1);
    } while (0);

    EasyIO::ByteBuffer buf;
    response.set_user_id(request.user_id());
    response.set_task_id(request.task_id());
    response.set_err_code(err.first);
    response.set_err_string(err.second);
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_READ_FILE_BLOCK, response, buf, aes())
            connection()->send(buf);
}

void FileTransmissionGuardAbility::handleCommandOfWriteFile(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(connection(), Global::Protocol::File::RequestWriteFileBlock, request, data, len, aes())

    std::pair<int, std::string> err;
    Global::Protocol::File::ResponseWriteFileBlock response;
    do
    {
        unsigned long l = request.raw_data_len();

        EasyIO::ByteBuffer serializedData(l);
        int zErr = uncompress((unsigned char*)serializedData.readableBytes(), &l,
                              (unsigned char*)request.compressed_data().c_str(), request.compressed_data().length());
        if (zErr != Z_OK)
        {
            Logger::warning("%s:%d - !uncompress, err %d, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__, zErr,
                            connection()->peerIP().c_str(), connection()->peerPort());
            err.first = GLOBAL_PROTOCOL_ERR_OPERATE_FAILED;
            err.second = "uncompress data failed";
            break;
        }

        err = FileTransmission::TaskScheduler::share()->writeLocalFileBlock(request.task_id(), request.seek(),
                            serializedData.readableBytes(), l);
        if (!err.first)
            response.set_after_seek(request.seek() + l);
    } while (false);


    EasyIO::ByteBuffer buf;
    response.set_user_id(request.user_id());
    response.set_task_id(request.task_id());
    response.set_err_code(err.first);
    response.set_err_string(err.second);
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_WRITE_FILE_BLOCK, response, buf, aes())
    connection()->send(buf);
}


void FileTransmissionGuardAbility::whenTaskAdded(const FileTransmission::Task &task)
{
    //do nothing
}

void FileTransmissionGuardAbility::whenTaskRemoved(const FileTransmission::Task &task)
{
    //do nothing
}

void FileTransmissionGuardAbility::whenTaskRemoteOpenFile(const FileTransmission::Task &task)
{
    //do nothing
}

void FileTransmissionGuardAbility::whenTaskRemoteCloseFile(const FileTransmission::Task &task)
{
    //do nothing
}

void FileTransmissionGuardAbility::whenTaskStateChanged(const FileTransmission::Task &task)
{
    //do nothing
}

void FileTransmissionGuardAbility::whenTaskProgressChanged(const FileTransmission::Task &task)
{
    //do nothing
}

void FileTransmissionGuardAbility::whenTaskRemoteWriteFileBlock(const FileTransmission::Task &task, unsigned long long progress, const char *data, size_t len)
{
    //do nothing
}

void FileTransmissionGuardAbility::whenTaskRemoteReadFileBlock(const FileTransmission::Task &task, unsigned long long progress, size_t len)
{
    //do nothing
}


void FileTransmissionGuardAbility::handleLoggedinEvent()
{

}

void FileTransmissionGuardAbility::handleConnectionLostEvent()
{

}

