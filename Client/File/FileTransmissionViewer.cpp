#include <quuid.h>
#include "FileTransmissionViewer.h"
#include "ui_FileTransmissionViewer.h"

#include <qapplication.h>
#include <qmessagebox.h>
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/File/File.pb.h"
#include "Global/Protocol/Error.h"
#include "Global/Component/Logger/Logger.h"
#include <string.h>
#include <zlib.h>
#include <zconf.h>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QAction>
#include <QSplitter>
#include <QGraphicsColorizeEffect>
#include <QDesktopServices>
#include "UsualParseProtobufDataMacro.h"
#include "Define.h"

#ifdef Q_OS_WIN
    #include <shlobj_core.h>
#endif

#define ROLE_TASK   (Qt::UserRole + 1)
#define KEY_TASK_ID     "KEY_TASK_ID"
#define KEY_TASK_STATE  "KEY_TASK_STATE"
#define COLUMN_TASK_ACTION          0
#define COLUMN_TASK_FROM_DIR        1
#define COLUMN_TASK_TO_DIR          2
#define COLUMN_TASK_FILE_NAME       3
#define COLUMN_TASK_FILE_SIZE       4
#define COLUMN_TASK_PROGRESS        5
#define COLUMN_TASK_CREATING_TIME   6
#define COLUMN_TASK_FINISHING_TIME  7
#define COLUMN_TASK_STATE           8
#define COLUMN_TASK_REASON          9
#define COLUMN_OF_ROLE_BIND     COLUMN_TASK_ACTION

#define PRECISION   2

#define COLOR_FAILED    (QColor(200, 0, 0))
#define COLOR_FINISHED  (QColor(0, 150, 0))

#define FILL_ENTRIES(_response, _dst) \
{ \
    _dst.reserve(4096); \
    for (int i = 0; i < _response.entries_size(); i++) \
    { \
        auto& e1 = _response.entries(i); \
        FileTransmission::EntryInfo e2; \
        e2.size = e1.size(); \
        e2.name = e1.name(); \
        e2.type = e1.is_directory() ? FileTransmission::EntryType::DIRECTORY : FileTransmission::EntryType::FILE; \
        e2.directory = e1.directory(); \
        e2.lastModifyingTime = QDateTime::fromMSecsSinceEpoch(e1.last_modifying_time()); \
        _dst.push_back(e2); \
    } \
}

inline QString formatProgress(double speed, size_t progress, size_t fileSize)
{
    static const double UNIT_MB = 1048576;
    static const double UNIT_KB = 1024;
    QString strSpeed;
    if (speed >= UNIT_MB)
    {
        strSpeed = QString::number(speed / UNIT_MB, 'f', 1) + " MB/s";
    }
    else
    {
        strSpeed = QString::number(speed / UNIT_KB, 'f', 1) + " KB/s";
    }

    QString txt = QString::number(((double)progress / fileSize) * 100, 'f', PRECISION) + "%";
    txt += "  " + strSpeed;
    return txt;
}


using namespace std::placeholders;

FileTransmissionViewer::FileTransmissionViewer(
        const std::string& peerName, const std::string& peerHost, const std::string& peerIP, unsigned short peerPort,
    const std::string& publicKey, const std::string& account, const std::string& password,
    User::ID partnerUserId, const std::string& partnerAlias, const std::string& authString, QWidget* parent)
    :
    QWidget(parent),
    ViewerBase(peerHost, peerIP, peerPort, publicKey, account, password, partnerUserId, authString),
    ui(new Ui::FileTransmissionViewer)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setWindowFlags(Qt::Window);

    QString title = QString::asprintf("%s[%s]-%s[%lld]", peerName.c_str(), peerHost.c_str(), partnerAlias.c_str(), partnerUserId);
    this->setWindowTitle(title);

    QAction *action;
    action = new QAction();
    action->setText("Pause");
    action->setIcon(QIcon(":/Resource/pause.png"));
    QObject::connect(action, &QAction::triggered,
        [this](bool checked)
        {
            stopSelectedTask();
        });
    ui->tblTasks->addAction(action);

    action = new QAction();
    action->setText("Resume");
    action->setIcon(QIcon(":/Resource/play.png"));
    QObject::connect(action, &QAction::triggered,
        [this](bool checked)
        {
            resumeSelectedTask();
        });
    ui->tblTasks->addAction(action);

    action = new QAction();
    action->setText("Remove");
    action->setIcon(QIcon(":/Resource/close.png"));
    QObject::connect(action, &QAction::triggered,
        [this](bool checked)
        {
            removeSelectedTask();
        });
    ui->tblTasks->addAction(action);

    action = new QAction();
    action->setText("Clean All Finished Tasks");
    action->setIcon(QIcon(":/Resource/clean.png"));
    QObject::connect(action, &QAction::triggered,
        [this](bool checked)
        {
            clearFinishedTasks();
        });
    ui->tblTasks->addAction(action);

    action = new QAction();
    action->setText("Show In Explorer");
    action->setIcon(QIcon(":/Resource/folder-open.png"));
    QObject::connect(action, &QAction::triggered,
        [this](bool checked)
        {
            showInExplorer();
        });
    ui->tblTasks->addAction(action);


    QObject::connect(this, SIGNAL(signalError(int,QString,bool)),
                    this, SLOT(handleError(int,QString,bool)));
    QObject::connect(this, SIGNAL(signalPartnerDefaultDirectories(QVector<std::string>)),
                    this, SLOT(partnerBrowserAddHistories(QVector<std::string>)));
    QObject::connect(this, SIGNAL(signalPartnerBrowserChangeCurrentPath(QString)),
                    this, SLOT(partnerBrowserChangeCurrentPath(QString)));
    QObject::connect(this, SIGNAL(signalPartnerBrowserLoadEntries(QVector<FileTransmission::EntryInfo>)),
                    this, SLOT(partnerBrowserLoadEntries(QVector<FileTransmission::EntryInfo>)));
    QObject::connect(this, SIGNAL(signalPartnerEntryDeleted(QString,QString)),
                    this, SLOT(partnerBrowserDeleteEntry(QString,QString)));
    QObject::connect(this, SIGNAL(signalPartnerDirectoryMade(QString, QString, QDateTime)),
                    this, SLOT(partnerBrowserMakeDirectory(QString, QString, QDateTime)));
    QObject::connect(this, SIGNAL(signalPartnerEntryRenamed(QString, QString, QString, QDateTime)),
                    this, SLOT(partnerBrowserRenameEntry(QString, QString, QString, QDateTime)));
    QObject::connect(this, SIGNAL(signalTaskAdded(FileTransmission::Task)),
                    this, SLOT(updateUITableForTaskAdded(FileTransmission::Task)));
    QObject::connect(this, SIGNAL(signalTaskRemoved(FileTransmission::Task)),
                    this, SLOT(updateUITableForTaskRemoved(FileTransmission::Task)));
    QObject::connect(this, SIGNAL(signalTaskStateChanged(FileTransmission::Task)),
                    this, SLOT(updateUITableForTaskStateChanged(FileTransmission::Task)));
    QObject::connect(this, SIGNAL(signalTaskProgressChanged(QString, uint64_t, uint64_t, double)),
                    this, SLOT(updateUITableForTaskProgressChanged(QString, uint64_t, uint64_t, double)));
    QObject::connect(this, SIGNAL(signalTaskOverwriteConsent(FileTransmission::Task)),
                    this, SLOT(fileOverwriteConsent(FileTransmission::Task)));


    QObject::connect(this, SIGNAL(signalLoggedIn()), this, SLOT(updateUISelfId()));
    QObject::connect(this, SIGNAL(signalPartnerAuthenticated()), this, SLOT(updateUIPartnerId()));


    m_selfBrowser = new FilesBrowseWidget(FilesBrowseWidget::UPLOAD);
    m_selfBrowser->onGoUp = std::bind(&FileTransmissionViewer::whenSelfGoUp, this, _1);
    m_selfBrowser->onGoHome = std::bind(&FileTransmissionViewer::whenSelfGoHome, this, _1);
    m_selfBrowser->onListEntries = std::bind(&FileTransmissionViewer::whenSelfListEntries, this, _1, _2);
    m_selfBrowser->onDeleteEntry = std::bind(&FileTransmissionViewer::whenSelfDeleteEntry, this, _1, _2);
    m_selfBrowser->onMakeDirectory = std::bind(&FileTransmissionViewer::whenSelfMakeDirectory, this, _1, _2);
    m_selfBrowser->onRenameEntry = std::bind(&FileTransmissionViewer::whenSelfRenameEntry, this, _1, _2, _3);
    m_selfBrowser->onTransmit = std::bind(&FileTransmissionViewer::whenSelfTransmit, this, _1, _2);
    ui->panelLeft->layout()->addWidget(m_selfBrowser);
    whenSelfGoHome(m_selfBrowser);

#ifdef Q_OS_WIN
    char path[256] = {0};
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, path)))
    {
        std::replace(path + 0, path + (sizeof(path) - 1), '\\', '/');
        m_selfBrowser->addDirectory(path);
        whenSelfListEntries(m_selfBrowser, path);
    }
    DWORD m = GetLogicalDrives();
    if (m)
    {
        for (int i = 0; i < sizeof(m) * 8 && i < 26; i++)
        {
            if (m & (1 << i))
            {
                m_selfBrowser->addDirectory(std::string(1, 'A' + i).append(":/"), 26);
            }
        }
    }
#else
    m_selfBrowser->addDirectory("/");
#endif

    m_partnerBrowser = new FilesBrowseWidget(FilesBrowseWidget::DOWNLOAD);
    m_partnerBrowser->onGoUp = std::bind(&FileTransmissionViewer::whenPartnerGoUp, this, _1);
    m_partnerBrowser->onGoHome = std::bind(&FileTransmissionViewer::whenPartnerGoHome, this, _1);
    m_partnerBrowser->onListEntries = std::bind(&FileTransmissionViewer::whenPartnerListEntries, this, _1, _2);
    m_partnerBrowser->onDeleteEntry = std::bind(&FileTransmissionViewer::whenPartnerDeleteEntry, this, _1, _2);
    m_partnerBrowser->onMakeDirectory = std::bind(&FileTransmissionViewer::whenPartnerMakeDirectory, this, _1, _2);
    m_partnerBrowser->onRenameEntry = std::bind(&FileTransmissionViewer::whenPartnerRenameEntry, this, _1, _2, _3);
    m_partnerBrowser->onTransmit = std::bind(&FileTransmissionViewer::whenPartnerTransmit, this, _1, _2);
    ui->panelRight->layout()->addWidget(m_partnerBrowser);

    FileTransmission::TaskScheduler::share()->subscribeEvents(this,
                std::bind(&FileTransmissionViewer::whenTaskAdded, this, _1),
                std::bind(&FileTransmissionViewer::whenTaskRemoved, this, _1),
                std::bind(&FileTransmissionViewer::whenTaskRemoteOpenFile, this, _1),
                std::bind(&FileTransmissionViewer::whenTaskRemoteCloseFile, this, _1),
                std::bind(&FileTransmissionViewer::whenTaskStateChanged, this, _1),
                std::bind(&FileTransmissionViewer::whenTaskProgressChanged, this, _1),
                std::bind(&FileTransmissionViewer::whenTaskRemoteWriteFileBlock, this, _1, _2, _3, _4),
                std::bind(&FileTransmissionViewer::whenTaskRemoteReadFileBlock, this, _1, _2, _3)
            );
}

FileTransmissionViewer::~FileTransmissionViewer()
{
    FileTransmission::TaskScheduler::share()->unsubscribeEvents(this);

    m_client->disconnect();
    while (m_client->connected())
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

    delete ui;
}


int FileTransmissionViewer::role()
{
    return GLOBAL_CONNECTION_ROLE_FILE_TRANSMISSION_VISITOR;
}

void FileTransmissionViewer::whenLoggedIn()
{
    Global::Protocol::File::RequestVisitFiles request;
    request.set_user_id(getPartnerUserId());
    request.set_auth_string(getAuthString());

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISIT_FILES, request, data, aes())
            send(data);
}

void FileTransmissionViewer::whenLoginFailed(int errCode, const std::string &errString)
{
    emit signalError(errCode, errString.c_str(), true);
}

bool FileTransmissionViewer::handleCompleteData(unsigned tag, const unsigned char *data, size_t len)
{
    switch (tag)
    {
    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISIT_FILES:
        handleResponseOfVisitFiles(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_MAKE_DIRECTORY :
        handleResponseOfMakeDirectory(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_DELETE_ENTRY :
        handleResponseOfDeleteEntry(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_RENAME_ENTRY :
        handleResponseOfRenameEntry(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISITING_PATH_GO_UP :
        handleResponseOfGoUp(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_VISITING_PATH_GO_HOME :
        handleResponseOfGoHome(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_LIST_ENTRIES :
        handleResponseOfListEntries(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_OPEN_FILE :
        handleResponseOfOpenFile(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_WRITE_FILE_BLOCK :
        handleResponseOfWriteFileBlock(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_READ_FILE_BLOCK :
        handleResponseOfReadFileBlock(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CLOSE_FILE :
        handleCommandOfCloseFile(data, len);
        return true;
    }
    return false;
}

void FileTransmissionViewer::handleResponseOfVisitFiles(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::File::ResponseVisitFiles, response, data, len, aes())

    if (response.err_code() != GLOBAL_PROTOCOL_ERR_NO_ERROR)
    {
        emit signalError(response.err_code(), response.err_string().c_str(), true);
    }
    else
    {
        QVector<FileTransmission::EntryInfo> entries;
        FILL_ENTRIES(response, entries)

        QVector<std::string> directories;
        for (int i = 0; i < response.default_dirs_size(); i++)
        {
            directories.push_back(response.default_dirs(i));
        }

        emit signalLoggedIn();
        emit signalPartnerAuthenticated();
        emit signalPartnerDefaultDirectories(directories);
        emit signalPartnerBrowserLoadEntries(entries);
        if (!directories.empty())
            emit signalPartnerBrowserChangeCurrentPath(directories.at(0).c_str());
    }
}

void FileTransmissionViewer::handleResponseOfGoUp(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::File::ResponseGoUp, response, data, len, aes())

    if (response.err_code() != GLOBAL_PROTOCOL_ERR_NO_ERROR)
    {
        emit signalError(response.err_code(), response.err_string().c_str(), false);
    }
    else
    {
        QVector<FileTransmission::EntryInfo> entries;
        FILL_ENTRIES(response, entries)

        emit signalPartnerBrowserChangeCurrentPath(response.dir().c_str());
        emit signalPartnerBrowserLoadEntries(entries);
    }
}

void FileTransmissionViewer::handleResponseOfGoHome(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::File::ResponseGoHome, response, data, len, aes())

    if (response.err_code() != GLOBAL_PROTOCOL_ERR_NO_ERROR)
    {
        emit signalError(response.err_code(), response.err_string().c_str(), false);
    }
    else
    {
        QVector<FileTransmission::EntryInfo> entries;
        FILL_ENTRIES(response, entries)

        emit signalPartnerBrowserChangeCurrentPath(response.dir().c_str());
        emit signalPartnerBrowserLoadEntries(entries);
    }
}

void FileTransmissionViewer::handleResponseOfListEntries(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::File::ResponseListEntries, response, data, len, aes())

    if (response.err_code() != GLOBAL_PROTOCOL_ERR_NO_ERROR)
    {
        emit signalError(response.err_code(), response.err_string().c_str(), false);
    }
    else
    {
        QVector<FileTransmission::EntryInfo> entries;
        FILL_ENTRIES(response, entries)

        emit signalPartnerBrowserChangeCurrentPath(response.dir().c_str());
        emit signalPartnerBrowserLoadEntries(entries);
    }
}

void FileTransmissionViewer::handleResponseOfDeleteEntry(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::File::ResponseDeleteEntry, response, data, len, aes())

    if (response.err_code() != GLOBAL_PROTOCOL_ERR_NO_ERROR)
    {
        emit signalError(response.err_code(), response.err_string().c_str(), false);
    }
    else
    {
        emit signalPartnerEntryDeleted(response.dir().c_str(), response.name().c_str());
    }
}

void FileTransmissionViewer::handleResponseOfMakeDirectory(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::File::ResponseMakeDirectory, response, data, len, aes())

    if (response.err_code() != GLOBAL_PROTOCOL_ERR_NO_ERROR)
    {
        emit signalError(response.err_code(), response.err_string().c_str(), false);
    }
    else
    {
        emit signalPartnerDirectoryMade(response.dir().c_str(), response.name().c_str(),
                    QDateTime::fromString(response.last_modifying_time().c_str(), GLOBAL_PROTOCOL_DATETIME_FORMAT));
    }
}

void FileTransmissionViewer::handleResponseOfRenameEntry(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::File::ResponseRenameEntry, response, data, len, aes())

    if (response.err_code() != GLOBAL_PROTOCOL_ERR_NO_ERROR)
    {
        emit signalError(response.err_code(), response.err_string().c_str(), false);
    }
    else
    {
        emit signalPartnerEntryRenamed(response.dir().c_str(), response.old_name().c_str(), response.new_name().c_str(),
                    QDateTime::fromString(response.last_modifying_time().c_str(), GLOBAL_PROTOCOL_DATETIME_FORMAT));
    }
}

void FileTransmissionViewer::handleResponseOfOpenFile(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::File::ResponseOpenFile, response, data, len, aes())

    if (!response.err_code())
    {
        FileTransmission::TaskScheduler::share()->remoteSuccessOpenFile(response.task_id());
    }
    else
    {
        FileTransmission::TaskScheduler::share()->remoteFailureOpenFile(
                    response.task_id(), response.err_code(), response.err_string());
    }
}

void FileTransmissionViewer::handleResponseOfReadFileBlock(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::File::ResponseReadFileBlock, response, data, len, aes())

    if (response.err_code() != GLOBAL_PROTOCOL_ERR_NO_ERROR)
    {
        FileTransmission::TaskScheduler::share()->failureReadRemoteFileBlock(
                    response.task_id(), response.err_code(), response.err_string());
    }
    else
    {
        unsigned long l = response.raw_data_len();

        EasyIO::ByteBuffer serializedData(l);
        int zErr = uncompress((unsigned char*)serializedData.readableBytes(), &l,
                              (unsigned char*)response.compressed_data().c_str(), response.compressed_data().length());
        if (zErr != Z_OK)
        {
            Logger::warning("%s:%d - !uncompress, err %d, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__, zErr,
                            m_client->peerIP().c_str(), m_client->peerPort());

            FileTransmission::TaskScheduler::share()->failureReadRemoteFileBlock(
                        response.task_id(), GLOBAL_PROTOCOL_ERR_OPERATE_FAILED, "uncompress failed");
            return;
        }

        FileTransmission::TaskScheduler::share()->successReadRemoteFileBlock(response.task_id(), response.seek(),
                            serializedData.readableBytes(), l);
    }
}

void FileTransmissionViewer::handleCommandOfCloseFile(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::File::CloseFile, request, data, len, aes())

    FileTransmission::TaskScheduler::share()->removeTask(request.task_id())
;}

void FileTransmissionViewer::handleResponseOfWriteFileBlock(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::File::ResponseWriteFileBlock, response, data, len, aes())

    if (response.err_code() != GLOBAL_PROTOCOL_ERR_NO_ERROR)
    {
        FileTransmission::TaskScheduler::share()->failureWriteRemoteFileBlock(
                    response.task_id(), response.err_code(), response.err_string());
    }
    else
    {
        FileTransmission::TaskScheduler::share()->successWriteRemoteFileBlock(response.task_id(), response.after_seek());
    }
}

void FileTransmissionViewer::whenConnectFailed(EasyIO::TCP::IConnection *con, const std::string &reason)
{
    emit signalError(GLOBAL_PROTOCOL_ERR_CONNECT_FAILED, reason.c_str(), true);
}

void FileTransmissionViewer::whenDisconnected(EasyIO::TCP::IConnection *con)
{
    emit signalError(GLOBAL_PROTOCOL_ERR_DISCONNECTED, "Disconnected", true);
}

void FileTransmissionViewer::whenSelfGoUp(FilesBrowseWidget *widget)
{
    if(m_dirSelf.cdUp())
    {
        QVector<FileTransmission::EntryInfo> entries;
        FileTransmission::getEntries(m_dirSelf, entries);
        m_selfBrowser->setCurrentDirectory(m_dirSelf.path().toStdString());
        m_selfBrowser->setEntries(entries);
    }
    else
    {
        QMessageBox::about(nullptr, "Go Up", "failed to go up");
    }
}

void FileTransmissionViewer::whenSelfGoHome(FilesBrowseWidget *widget)
{
    if(m_dirSelf.cd(m_dirSelf.homePath()))
    {
        QVector<FileTransmission::EntryInfo> entries;
        FileTransmission::getEntries(m_dirSelf, entries);
        m_selfBrowser->setCurrentDirectory(m_dirSelf.path().toStdString());
        m_selfBrowser->setEntries(entries);
    }
    else
    {
        QMessageBox::about(nullptr, "Go Home", "failed to go home");
    }
}

void FileTransmissionViewer::whenSelfListEntries(FilesBrowseWidget *widget, const std::string &directory)
{
    if(m_dirSelf.cd(directory.c_str()))
    {
        QVector<FileTransmission::EntryInfo> entries;
        FileTransmission::getEntries(m_dirSelf, entries);
        m_selfBrowser->setCurrentDirectory(m_dirSelf.path().toStdString());
        m_selfBrowser->setEntries(entries);
    }
    else
    {
        QMessageBox::about(nullptr, "List entries", "failed to list entries");
    }
}

void FileTransmissionViewer::whenSelfDeleteEntry(FilesBrowseWidget *widget, const FileTransmission::EntryInfo &entry)
{
    bool b;
    if (entry.type == FileTransmission::EntryType::FILE)
    {
        b = m_dirSelf.remove(entry.name.c_str());
    }
    else
    {
        QDir dir(m_dirSelf);
        b = dir.cd(entry.name.c_str()) && dir.removeRecursively();
    }

    if (b)
        m_selfBrowser->removeEntry(entry);
    else
      QMessageBox::about(nullptr, "Delete entry", "failed to delete entry");
}

void FileTransmissionViewer::whenSelfMakeDirectory(FilesBrowseWidget *, const std::string &directory)
{
    FileTransmission::EntryInfo entry;
    entry.name = directory;
    entry.size = 0;
    entry.type = FileTransmission::EntryType::DIRECTORY;
    entry.directory = m_dirSelf.path().toStdString();
    entry.lastModifyingTime = QDateTime::currentDateTime();
    if (m_dirSelf.mkdir(directory.c_str()))
    {
        m_selfBrowser->appendEntry(entry);
    }
}

void FileTransmissionViewer::whenSelfRenameEntry(FilesBrowseWidget *, const std::string &oldName, const std::string &newName)
{
    if (m_dirSelf.rename(oldName.c_str(), newName.c_str()))
    {
        m_selfBrowser->renameEntry(oldName.c_str(), newName.c_str(), QDateTime::currentDateTime());
    }
    else
    {
        QMessageBox::about(nullptr, "Delete entry", "failed to rename entry");
    }
}

void FileTransmissionViewer::whenSelfTransmit(FilesBrowseWidget *w, const FileTransmission::EntryInfo &entry)
{
    FileTransmission::Task task;
    task.serverHost = peerHost();
    task.serverPort = peerPort();
    task.partnerId = getPartnerUserId();
    task.starter = FileTransmission::Task::Starter::SELF;
    task.action = FileTransmission::Task::Action::WRITE;
    task.fromDirectory = entry.directory;
    task.toDirectory = m_partnerBrowser->getCurrentDirectory();
    task.fileName = entry.name;
    task.progress = 0;
    task.fileSize = entry.size;
    task.overrideFile = false;

    auto err = FileTransmission::TaskScheduler::share()->addTaskActively(task.serverHost, task.serverPort,
        task.partnerId, task.action, task.fromDirectory, task.toDirectory,
        task.fileName, task.progress, task.fileSize, task.overrideFile);
    if (err.first)
    {
        emit signalError(err.first, err.second.c_str(), false);
    }
}

void FileTransmissionViewer::whenPartnerGoUp(FilesBrowseWidget *widget)
{
    Global::Protocol::File::RequestGoUp request;
    request.set_seq(++m_seq);
    request.set_dir(m_partnerBrowser->getCurrentDirectory());

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISITING_PATH_GO_UP, request, data, aes())
    send(data);
}

void FileTransmissionViewer::whenPartnerGoHome(FilesBrowseWidget *widget)
{
    Global::Protocol::File::RequestGoHome request;
    request.set_seq(++m_seq);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_VISITING_PATH_GO_HOME, request, data, aes())
    send(data);
}

void FileTransmissionViewer::whenPartnerEnterDirectory(FilesBrowseWidget *widget, const std::string &directory)
{
    whenPartnerListEntries(widget, directory);
}

void FileTransmissionViewer::whenPartnerListEntries(FilesBrowseWidget *widget, const std::string &directory)
{
    Global::Protocol::File::RequestListEntries request;
    request.set_seq(++m_seq);
    request.set_dir(directory);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_LIST_ENTRIES, request, data, aes())
    send(data);
}

void FileTransmissionViewer::whenPartnerDeleteEntry(FilesBrowseWidget *widget, const FileTransmission::EntryInfo &entry)
{
    Global::Protocol::File::RequestDeleteEntry request;
    request.set_seq(++m_seq);
    request.set_dir(entry.directory);
    request.set_name(entry.name);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_DELETE_ENTRY, request, data, aes())
    send(data);
}

void FileTransmissionViewer::whenPartnerMakeDirectory(FilesBrowseWidget *, const std::string &directory)
{
    Global::Protocol::File::RequestMakeDirectory request;
    request.set_seq(++m_seq);
    request.set_dir(m_partnerBrowser->getCurrentDirectory());
    request.set_name(directory);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_MAKE_DIRECTORY, request, data, aes())
    send(data);
}

void FileTransmissionViewer::whenPartnerRenameEntry(FilesBrowseWidget *, const std::string &oldName, const std::string &newName)
{
    Global::Protocol::File::RequestRenameEntry request;
    request.set_seq(++m_seq);
    request.set_dir(m_partnerBrowser->getCurrentDirectory());
    request.set_old_name(oldName);
    request.set_new_name(newName);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_RENAME_ENTRY, request, data, aes())
            send(data);
}

void FileTransmissionViewer::whenPartnerTransmit(FilesBrowseWidget *w, const FileTransmission::EntryInfo &entry)
{
    FileTransmission::Task task;
    task.serverHost = peerHost();
    task.serverPort = peerPort();
    task.partnerId = getPartnerUserId();
    task.starter = FileTransmission::Task::Starter::SELF;
    task.action = FileTransmission::Task::Action::READ;
    task.fromDirectory = entry.directory;
    task.toDirectory = m_selfBrowser->getCurrentDirectory();
    task.fileName = entry.name;
    task.progress = 0;
    task.fileSize = entry.size;
    task.overrideFile = false;

    auto err = FileTransmission::TaskScheduler::share()->addTaskActively(task.serverHost, task.serverPort,
        task.partnerId, task.action, task.fromDirectory, task.toDirectory,
        task.fileName, task.progress, task.fileSize, task.overrideFile);
    if (err.first)
    {
        emit signalError(err.first, err.second.c_str(), false);
    }
}

void FileTransmissionViewer::updateUITableForTaskAdded(FileTransmission::Task task)
{
    int r = 0;
    ui->tblTasks->insertRow(r);
    ui->tblTasks->setItem(r, COLUMN_TASK_ACTION, new QTableWidgetItem(FileTransmission::Task::toString(task.action)));
    ui->tblTasks->setItem(r, COLUMN_TASK_FROM_DIR, new QTableWidgetItem(task.fromDirectory.c_str()));
    ui->tblTasks->setItem(r, COLUMN_TASK_TO_DIR, new QTableWidgetItem(task.toDirectory.c_str()));
    ui->tblTasks->setItem(r, COLUMN_TASK_FILE_NAME, new QTableWidgetItem(task.fileName.c_str()));
    ui->tblTasks->setItem(r, COLUMN_TASK_FILE_SIZE, new QTableWidgetItem(FilesBrowseWidget::formatSize(task.fileSize)));
    ui->tblTasks->setItem(r, COLUMN_TASK_PROGRESS, new QTableWidgetItem(
                              QString::number(((double)task.progress / task.fileSize) * 100, 'f', PRECISION) + "%"));
    ui->tblTasks->setItem(r, COLUMN_TASK_CREATING_TIME, new QTableWidgetItem(task.creatingTime.toString(GLOBAL_PROTOCOL_DATETIME_FORMAT)));
    ui->tblTasks->setItem(r, COLUMN_TASK_FINISHING_TIME, new QTableWidgetItem(
                              task.state == FileTransmission::Task::State::FINISHED ?
                                  task.finishingTime.toString(GLOBAL_PROTOCOL_DATETIME_FORMAT) : ""));
    ui->tblTasks->setItem(r, COLUMN_TASK_STATE, new QTableWidgetItem(FileTransmission::Task::toString(task.state)));
    if (task.state == FileTransmission::Task::State::FAILED)
        ui->tblTasks->item(r, COLUMN_TASK_STATE)->setForeground(COLOR_FAILED);
    else if (task.state == FileTransmission::Task::State::FINISHED)
        ui->tblTasks->item(r, COLUMN_TASK_STATE)->setForeground(COLOR_FINISHED);
    ui->tblTasks->setItem(r, COLUMN_TASK_REASON, new QTableWidgetItem(
                              task.state == FileTransmission::Task::State::FINISHED ? task.reason.c_str() : ""));

    QVariantMap m;
    m[KEY_TASK_ID] = task.id.c_str();
    m[KEY_TASK_STATE] = (int)task.state;
    ui->tblTasks->item(r, COLUMN_OF_ROLE_BIND)->setData(ROLE_TASK, m);

    for (int i = 0; i < ui->tblTasks->columnCount(); i++)
    {
        auto it = ui->tblTasks->item(0, i);
        it->setToolTip(it->text());
    }
}

void FileTransmissionViewer::updateUITableForTaskRemoved(FileTransmission::Task task)
{
    for (int r = 0; r < ui->tblTasks->rowCount(); r++)
    {
        auto m = ui->tblTasks->item(r, COLUMN_OF_ROLE_BIND)->data(ROLE_TASK).toMap();
        std::string taskId = m[KEY_TASK_ID].toString().toStdString();
        if (taskId == task.id)
        {
            ui->tblTasks->removeRow(r);
            return;
        }
    }
}

#define CHECK_CALLBACK_EVENT(_task)  \
    if (task.starter != FileTransmission::Task::Starter::SELF) \
        return; \
    \
    if (task.serverHost != peerHost() || task.serverPort != peerPort()) \
        return;

void FileTransmissionViewer::whenTaskAdded(const FileTransmission::Task &task)
{
    CHECK_CALLBACK_EVENT(task)
    emit signalTaskAdded(task);
}

void FileTransmissionViewer::whenTaskRemoved(const FileTransmission::Task &task)
{
    CHECK_CALLBACK_EVENT(task)
    emit signalTaskRemoved(task);
}

void FileTransmissionViewer::whenTaskRemoteOpenFile(const FileTransmission::Task &task)
{
    assert(task.starter == FileTransmission::Task::Starter::SELF);

    Global::Protocol::File::RequestOpenFile request;
    request.set_seq(++m_seq);
    request.set_user_id(m_selfUserId);
    request.set_task_id(task.id);
    request.set_mode((int)task.action);
    request.set_directory(task.action == FileTransmission::Task::Action::READ ?
                              task.fromDirectory : task.toDirectory);
    request.set_file_name(task.fileName);
    request.set_file_size(task.fileSize);
    request.set_seek(task.progress);
    request.set_force(task.overrideFile);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_OPEN_FILE, request, data, aes())
    send(data);
}

void FileTransmissionViewer::whenTaskRemoteCloseFile(const FileTransmission::Task &task)
{
    Global::Protocol::File::CloseFile request;
    request.set_user_id(m_selfUserId);
    request.set_task_id(task.id);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CLOSE_FILE, request, data, aes())
    send(data);
}

void FileTransmissionViewer::whenTaskStateChanged(const FileTransmission::Task &task)
{
    CHECK_CALLBACK_EVENT(task)
    emit signalTaskStateChanged(task);

    if (task.state == FileTransmission::Task::State::FAILED)
    {
        if (task.error == GLOBAL_PROTOCOL_ERR_FILE_ALREADY_EXISTS)
            emit signalTaskOverwriteConsent(task);
    }
}

void FileTransmissionViewer::whenTaskProgressChanged(const FileTransmission::Task &task)
{
    CHECK_CALLBACK_EVENT(task)
    emit signalTaskProgressChanged(task.id.c_str(), task.progress, task.fileSize, task.speed);
}

void FileTransmissionViewer::whenTaskRemoteWriteFileBlock(const FileTransmission::Task &task, unsigned long long progress, const char *data, size_t len)
{
    unsigned long l1 = compressBound(len);
    EasyIO::ByteBuffer compressedData(l1);
    int zErr = compress((unsigned char*)compressedData.readableBytes(), &l1,
                        (unsigned char*)data, len);
    if (zErr != Z_OK)
    {
        Logger::warning("%s:%d - !compress, error %d, remote addres %s:%d", __PRETTY_FUNCTION__, __LINE__, zErr,
                        m_client->peerIP().c_str(), m_client->peerPort());
        FileTransmission::TaskScheduler::share()->failureWriteRemoteFileBlock(
                    task.id, GLOBAL_PROTOCOL_ERR_OPERATE_FAILED, "compress data failed");
        return;
    }

    EasyIO::ByteBuffer buf;
    Global::Protocol::File::RequestWriteFileBlock request;
    request.set_user_id(task.partnerId);
    request.set_task_id(task.id);
    request.set_seek(progress);
    request.set_raw_data_len(len);
    request.set_compressed_data(compressedData.readableBytes(), l1);
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_WRITE_FILE_BLOCK, request, buf, aes())
    send(buf);
}

void FileTransmissionViewer::whenTaskRemoteReadFileBlock(const FileTransmission::Task &task, unsigned long long progress, size_t len)
{
    EasyIO::ByteBuffer buf;
    Global::Protocol::File::RequestReadFileBlock request;
    request.set_user_id(task.partnerId);
    request.set_task_id(task.id);
    request.set_seek(progress);
    request.set_len(len);
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_READ_FILE_BLOCK, request, buf, aes())
    send(buf);
}


void FileTransmissionViewer::updateUITableForTaskProgressChanged(QString taskId, uint64_t progress, uint64_t fileSize, double speed)
{
    for (int i = 0; i < ui->tblTasks->rowCount(); i++)
    {
        auto id = ui->tblTasks->item(i, COLUMN_OF_ROLE_BIND)->data(ROLE_TASK).toMap()[KEY_TASK_ID].toString();
        if (id == taskId)
        {
            QString txt = formatProgress(speed, progress, fileSize);
            ui->tblTasks->item(i, COLUMN_TASK_PROGRESS)->setText(txt);
            return;
        }
    }
}

void FileTransmissionViewer::updateUISelfId()
{
    ui->lblLocalId->setText(QString::number(this->getSelfUserId()));
}

void FileTransmissionViewer::updateUIPartnerId()
{
    ui->lblPartnerId->setText(QString::number(this->getPartnerUserId()));
}

void FileTransmissionViewer::stopSelectedTask()
{
    int row = ui->tblTasks->currentRow();
    if (row == -1)
    {
        QMessageBox::about(nullptr, "failed", "No rows are selected");
        return;
    }

    auto it = ui->tblTasks->item(row, COLUMN_OF_ROLE_BIND);
    QVariantMap m = it->data(ROLE_TASK).toMap();
    auto id = m[KEY_TASK_ID].toString().toStdString();

    FileTransmission::TaskScheduler::share()->stopTask(id);
}

void FileTransmissionViewer::resumeSelectedTask()
{
    int row = ui->tblTasks->currentRow();
    if (row == -1)
    {
        QMessageBox::about(nullptr, "failed", "No rows are selected");
        return;
    }

    auto it = ui->tblTasks->item(row, COLUMN_OF_ROLE_BIND);
    QVariantMap m = it->data(ROLE_TASK).toMap();
    auto id = m[KEY_TASK_ID].toString().toStdString();

    FileTransmission::TaskScheduler::share()->resumeTask(id);
}

void FileTransmissionViewer::removeSelectedTask()
{
    int row = ui->tblTasks->currentRow();
    if (row == -1)
    {
        QMessageBox::about(nullptr, "failed", "No rows are selected");
        return;
    }

    auto it = ui->tblTasks->item(row, COLUMN_OF_ROLE_BIND);
    QVariantMap m = it->data(ROLE_TASK).toMap();
    auto id = m[KEY_TASK_ID].toString().toStdString();

    FileTransmission::TaskScheduler::share()->removeTask(id);
}

void FileTransmissionViewer::clearFinishedTasks()
{
    std::vector<QString> ids;
    for (int r = 0; r < ui->tblTasks->rowCount(); r++)
    {
        QMap m = ui->tblTasks->item(r, COLUMN_OF_ROLE_BIND)->data(ROLE_TASK).toMap();
        QString taskId = m[KEY_TASK_ID].toString();
        FileTransmission::Task::State state = (FileTransmission::Task::State)m[KEY_TASK_STATE].toInt();
        if (state == FileTransmission::Task::State::FINISHED)
            ids.push_back(taskId);
    }

    for (auto id : ids)
        FileTransmission::TaskScheduler::share()->removeTask(id.toStdString());
}

void FileTransmissionViewer::showInExplorer()
{
    int row = ui->tblTasks->currentRow();
    if (row == -1)
    {
        QMessageBox::about(nullptr, "failed", "No rows are selected");
        return;
    }

    QString action = ui->tblTasks->item(row, COLUMN_TASK_ACTION)->text();
    QString from = ui->tblTasks->item(row, COLUMN_TASK_FROM_DIR)->text();
    QString to = ui->tblTasks->item(row, COLUMN_TASK_TO_DIR)->text();
    QString path = from;
    if (action == FileTransmission::Task::toString(FileTransmission::Task::Action::READ))
        path = to;

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void FileTransmissionViewer::fileOverwriteConsent(FileTransmission::Task task)
{
    if(QMessageBox::question(this, "Overwrite File",
            ("The file " + task.fileName + " already exists. Do you want to overwrite it?").c_str(),
            QMessageBox::Button::Yes | QMessageBox::Button::No, QMessageBox::Button::No)
        == QMessageBox::Button::Yes)
    {
        FileTransmission::TaskScheduler::share()->removeTask(task.id);

        auto err = FileTransmission::TaskScheduler::share()->addTaskActively(task.serverHost, task.serverPort,
            task.partnerId, task.action, task.fromDirectory, task.toDirectory,
            task.fileName, task.progress, task.fileSize, true);
        if (err.first)
        {
            emit signalError(err.first, err.second.c_str(), false);
        }
    }
}

void FileTransmissionViewer::updateUITableForTaskStateChanged(FileTransmission::Task task)
{
    for (int i = 0; i < ui->tblTasks->rowCount(); i++)
    {
        auto it = ui->tblTasks->item(i, COLUMN_OF_ROLE_BIND);
        QVariantMap m = it->data(ROLE_TASK).toMap();
        auto id = m[KEY_TASK_ID].toString().toStdString();
        if (id == task.id)
        {
            QTableWidgetItem* item;
            item = ui->tblTasks->item(i, COLUMN_TASK_STATE);
            item->setText(FileTransmission::Task::toString(task.state));
            item->setToolTip(item->text());
            if (task.state == FileTransmission::Task::State::FAILED)
                item->setForeground(COLOR_FAILED);
            else if (task.state == FileTransmission::Task::State::FINISHED)
                item->setForeground(COLOR_FINISHED);

            item = ui->tblTasks->item(i, COLUMN_TASK_PROGRESS);
            item->setText(formatProgress(task.speed, task.progress, task.fileSize));
            item->setToolTip(item->text());

            if (task.state == FileTransmission::Task::State::FINISHED)
            {
                item = new QTableWidgetItem(task.finishingTime.toString(GLOBAL_PROTOCOL_DATETIME_FORMAT));
                item->setToolTip(item->text());
                ui->tblTasks->setItem(i, COLUMN_TASK_FINISHING_TIME, item);
            }
            else if (task.state == FileTransmission::Task::State::FAILED)
            {
                item = new QTableWidgetItem(task.reason.c_str());
                item->setToolTip(item->text());
                ui->tblTasks->setItem(i, COLUMN_TASK_REASON, item);
            }

            m[KEY_TASK_STATE] = (int)task.state;
            it->setData(ROLE_TASK, m);
            break;
        }
    }

    if (task.state == FileTransmission::Task::State::FINISHED)
    {
        if (task.action == FileTransmission::Task::Action::READ)    //doanload
        {
            if (task.toDirectory == m_selfBrowser->getCurrentDirectory())
            {
                FileTransmission::EntryInfo entry;
                entry.directory = task.toDirectory;
                entry.lastModifyingTime = task.finishingTime;
                entry.name = task.fileName;
                entry.size = task.fileSize;
                entry.type = FileTransmission::EntryType::FILE;
                m_selfBrowser->appendEntry(entry);
            }
        }
        else    //upload
        {
            if (task.toDirectory == m_partnerBrowser->getCurrentDirectory())
            {
                FileTransmission::EntryInfo entry;
                entry.directory = task.toDirectory;
                entry.lastModifyingTime = task.finishingTime;
                entry.name = task.fileName;
                entry.size = task.fileSize;
                entry.type = FileTransmission::EntryType::FILE;
                m_partnerBrowser->appendEntry(entry);
            }
        }
    }
}

void FileTransmissionViewer::handleError(int errCode, QString errString, bool fatal)
{
    QMessageBox::about(this, "error", QString::asprintf("err[%d]: %s", errCode, errString.toStdString().c_str()));
    if (fatal)
        this->close();
}

void FileTransmissionViewer::partnerBrowserAddHistories(QVector<std::string> directories)
{
    for (int i = directories.size() - 1; i >= 0; i--)
        m_partnerBrowser->addDirectory(directories[i]);
}

void FileTransmissionViewer::partnerBrowserChangeCurrentPath(QString path)
{
    m_partnerBrowser->setCurrentDirectory(path.toStdString());
}

void FileTransmissionViewer::partnerBrowserLoadEntries(QVector<FileTransmission::EntryInfo> entries)
{
    m_partnerBrowser->setEntries(entries);
}

void FileTransmissionViewer::partnerBrowserDeleteEntry(QString directory, QString name)
{
    if (m_partnerBrowser->getCurrentDirectory() == directory.toStdString())
    {
        m_partnerBrowser->removeEntry(name.toStdString());
    }
}

void FileTransmissionViewer::partnerBrowserRenameEntry(QString directory, QString oldName, QString newName, QDateTime lastModifyingTime)
{
    if (m_partnerBrowser->getCurrentDirectory() == directory.toStdString())
    {
        m_partnerBrowser->renameEntry(oldName.toStdString(), newName.toStdString(), lastModifyingTime);
    }
}

void FileTransmissionViewer::partnerBrowserMakeDirectory(QString directory, QString name, QDateTime lastModifyingTime)
{
    if (m_partnerBrowser->getCurrentDirectory() == directory.toStdString())
    {
        FileTransmission::EntryInfo entry;
        entry.name = name.toStdString();
        entry.size = 0;
        entry.type = FileTransmission::EntryType::DIRECTORY;
        entry.directory = directory.toStdString();
        entry.lastModifyingTime = lastModifyingTime;
        m_partnerBrowser->appendEntry(entry);
    }
}
