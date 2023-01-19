#include "FileTransmission.h"
#include <quuid.h>
#include <assert.h>
#include <qdir.h>
#include <limits>
#include "Global/Component/Logger/Logger.h"
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qvariant.h>
#include <QDir>
#include <qcoreapplication.h>
#include "Global/Define.h"
#include "Global/Protocol/Error.h"
#include "Global/Protocol/Protocol.h"

#define DB_DIR                      (qApp->applicationDirPath() + "/data")
#define DB_PATH                     (DB_DIR + "/FileTransmission.db")
#define TABLE_TASK   "task_v_0_1"
#define TIME_OUT    60
#define BLOCK_SIZE  65536
#define PROGRESS_FLUSH_INTERVAL  1000

FileTransmission::TaskScheduler* FileTransmission::TaskScheduler::s_this = nullptr;

void FileTransmission::getEntries(const QDir &dir, QVector<EntryInfo> &dst)
{
    QFileInfoList list = dir.entryInfoList();
    for (auto it = list.begin(); it != list.end(); it++)
    {
        if (it->fileName() == ".")
            continue;
        if (dir.isRoot() && it->fileName() == "..")
            continue;

        FileTransmission::EntryInfo entry;
        entry.name = it->fileName().toStdString();
        entry.size = it->size();
        entry.type = it->isDir() ? FileTransmission::EntryType::DIRECTORY : FileTransmission::EntryType::FILE;
        entry.directory = it->path().toStdString();
        entry.lastModifyingTime = it->lastModified();
        dst.push_back(entry);
    }
}

const char *FileTransmission::Task::toString(State state)
{
    switch (state)
    {
    case State::WAITING_FOR_START:
        return "Waiting For Start";

    case State::PREPARING:
        return "Preparing";

    case State::TRANSMITTING:
        return "Transmitting";

    case State::STOPED:
        return "Stoped";

    case State::FINISHED:
        return "Finished";

    case State::FAILED:
        return "Failed";

    default:
        return "";
    }
}

const char *FileTransmission::Task::toString(Action action)
{
    switch (action)
    {
    case Action::WRITE:
        return "Upload";

    case Action::READ:
        return "Download";

    default:
        assert(0);
    }
}

FileTransmission::TaskScheduler *FileTransmission::TaskScheduler::share()
{
    return s_this;
}

bool FileTransmission::TaskScheduler::init()
{
    assert(s_this == nullptr);

    do
    {
        s_this = new TaskScheduler();
        if (!s_this->initDB() || !s_this->initTable())
            break;

        s_this->loadTasks();
        s_this->m_thread = std::thread(std::bind(&FileTransmission::TaskScheduler::exec, s_this));
        return true;
    }
    while (false);

    delete s_this;
    s_this = nullptr;
    return false;
}

void FileTransmission::TaskScheduler::release()
{
    if (s_this)
    {
        s_this->exit();
        s_this->waitingForExited();
        delete s_this;
        s_this = nullptr;
    }
}

void FileTransmission::TaskScheduler::exit()
{
    m_exit = true;
}

void FileTransmission::TaskScheduler::waitingForExited()
{
    if (m_thread.joinable())
            m_thread.join();
}

std::pair<int, std::string> FileTransmission::TaskScheduler::addTaskActively(
                const std::string &serverHost, unsigned short serverPort,
                User::ID partnerId,  Task::Action action, const std::string &fromDirectory,
                const std::string &toDirectory, const std::string &fileName, uint64_t progress, uint64_t fileSize, bool overrideFile)
{
    return addTask(QUuid::createUuid().toString(QUuid::Id128).toStdString(), serverHost, serverPort, partnerId,
                   Task::Starter::SELF, action, Task::State::WAITING_FOR_START, fromDirectory, toDirectory, fileName,
                   progress, fileSize, "", overrideFile);
}

std::pair<int, std::string> FileTransmission::TaskScheduler::addTaskPassively(const std::string &id, const std::string &serverHost, unsigned short serverPort,
                User::ID partnerId, Task::Action action, const std::string &directory,
                const std::string &fileName, uint64_t progress, uint64_t fileSize, bool overrideFile)
{
    std::string fromDirectory, toDirectory;
    if (action == Task::Action::WRITE)
        toDirectory = directory;
    else
        fromDirectory = directory;

    return addTask(id, serverHost, serverPort, partnerId,
                   Task::Starter::PARTNER, action, Task::State::TRANSMITTING, fromDirectory, toDirectory, fileName,
                   progress, fileSize, "", overrideFile);
}

std::pair<int, std::string> FileTransmission::TaskScheduler::addTask(const std::string &id, const std::string &serverHost, unsigned short serverPort,
                User::ID partnerId, Task::Starter starter, Task::Action action, Task::State state, const std::string &fromDirectory,
                const std::string &toDirectory, const std::string &fileName, uint64_t progress, uint64_t fileSize,
                const std::string& reason, bool overrideFile)
{
    QDateTime now = QDateTime::currentDateTime();
    TaskPrivatePtr task(new TaskPrivate());
    task->serverHost = serverHost;
    task->serverPort = serverPort;
    task->fp = nullptr;
    task->id = id;
    task->fileName = fileName;
    task->fileSize = fileSize;
    task->creatingTime = now;
    task->action = action;
    task->state = state;
    task->starter = starter;
    task->progress = progress;
    task->speed = 0;
    task->partnerId = partnerId;
    task->toDirectory = toDirectory;
    task->fromDirectory = fromDirectory;
    task->lastActivatingTime = QDateTime::currentDateTime();
    task->error = 0;
    task->reason = reason;
    task->overrideFile = overrideFile;

    if (starter == Task::Starter::PARTNER)
    {
        assert(state == Task::State::TRANSMITTING);
        std::pair<int, std::string> ret = task->initIO(overrideFile);
        if (ret.first)
            return ret;
    }

    std::pair<int, std::string> ret;
    {
        std::lock_guard g(m_mutex);
        if(!m_tasks.insert({task->id, task}).second)
        {
            ret.first = GLOBAL_PROTOCOL_ERR_OPERATE_FAILED;
            ret.second = "Task " + id + "  already exists";
            return ret;
        }
        m_queuedLocalTasks.push_back(task);
    }
    notifyTaskAdded(*task);
    return ret;
}

bool FileTransmission::TaskScheduler::subscribeEvents(void *object,
                                                      std::function<void(const Task& task)> taskAddedCallback,
                                                      std::function<void(const Task& taskId)> taskRemovedCallback,
                                                      std::function<void(const Task& task)> remoteOpenFileCallback,
                                                      std::function<void(const Task& task)> remoteCloseFileCallback,
                                                      std::function<void(const Task& task)> stateChangedCallback,
                                                      std::function<void(const Task& task)> progressChangedCallback,
                                                      std::function<void(const Task& task, unsigned long long progress, const char* data, size_t len)> remoteWriteFileBlockCallback,
                                                      std::function<void(const Task& task, unsigned long long progress, size_t len)> remoteReadFileBlockCallback)
{
    if (!taskAddedCallback || !taskRemovedCallback || !stateChangedCallback
                || !remoteOpenFileCallback || !remoteCloseFileCallback || !progressChangedCallback
                || !remoteWriteFileBlockCallback || !remoteReadFileBlockCallback)
            return false;

        Callbacks callbacks;
        callbacks.taskAddedCallback = taskAddedCallback;
        callbacks.taskRemovedCallback = taskRemovedCallback;
        callbacks.stateChangedCallback = stateChangedCallback;
        callbacks.remoteOpenFileCallback = remoteOpenFileCallback;
        callbacks.remoteCloseFileCallback = remoteCloseFileCallback;
        callbacks.progressChangedCallback = progressChangedCallback;
        callbacks.remoteWriteFileBlockCallback = remoteWriteFileBlockCallback;
        callbacks.remoteReadFileBlockCallback = remoteReadFileBlockCallback;

        std::lock_guard g(m_mutex);
        m_followers[object] = callbacks;

        for (auto it : m_tasks)
        {
            std::lock_guard g2(it.second->mutex);
            taskAddedCallback(*it.second);
        }

        return true;
}

void FileTransmission::TaskScheduler::unsubscribeEvents(void *object)
{
    std::lock_guard g(m_mutex);
    m_followers.erase(object);
}

void FileTransmission::TaskScheduler::removeTask(const std::string &taskId)
{
    auto remove = [](std::list<TaskPrivatePtr>& tasks, const std::string &taskId)
    {
        for (auto it = tasks.begin(); it != tasks.end(); it++)
        {
            if ((*it)->id == taskId)
            {
                tasks.erase(it);
                Logger::info("%s:%d - task %s removed",  __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
                return;
            }
        }
    };

    TaskPrivatePtr task;
    {
        std::lock_guard g(m_mutex);
        auto it = m_tasks.find(taskId);
        if (it != m_tasks.end())
            task = it->second;
        m_tasks.erase(taskId);
        remove(m_queuedLocalTasks, taskId);
    }
    if (task.get())
    {
        std::lock_guard g(task->mutex);
        notifyTaskRemoved(*task);
    }
}

void FileTransmission::TaskScheduler::stopTask(const std::string &taskId)
{
    TaskPrivatePtr task = getTask0(taskId);
    if (!task.get())
    {
        Logger::info("%s:%d - task %s does not exist",
                     __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
        return;
    }

    {
        std::lock_guard g(task->mutex);
        if (task->starter != Task::Starter::SELF || task->state != Task::State::TRANSMITTING)
        {
            Logger::info("%s:%d - stop task %s failed: starter != SELF || state != TRANSMITTING",
                         __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
            return;
        }
        task->state = Task::State::STOPED;
        task->lastActivatingTime = QDateTime::currentDateTime();
    }
    notifyTaskRemoteCloseFile(*task);
    notifyTaskStateChanged(*task);
}

void FileTransmission::TaskScheduler::resumeTask(const std::string &taskId)
{
    TaskPrivatePtr task = getTask0(taskId);
    if (!task.get())
    {
        Logger::info("%s:%d - task %s does not exist",
                     __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
        return;
    }
    {
        std::lock_guard g(task->mutex);
        if (task->starter != Task::Starter::SELF || task->state != Task::State::STOPED)
        {
            Logger::info("%s:%d - resume task %s failed: starter != SELF || state != STOPED",
                         __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
            return;
        }
        task->state = Task::State::WAITING_FOR_START;
        task->lastActivatingTime = QDateTime::currentDateTime();
    }
    notifyTaskStateChanged(*task);

    std::lock_guard g(m_mutex);
    m_queuedLocalTasks.push_back(task);
}

void FileTransmission::TaskScheduler::remoteSuccessOpenFile(const std::string& taskId)
{
    TaskPrivatePtr task = getTask0(taskId);
    if (!task.get())
    {
        Logger::info("%s:%d - task %s does not exist",
                     __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
        return;
    }
    {
        std::lock_guard g(task->mutex);
        if (task->state != Task::State::PREPARING)
        {
            Logger::info("%s:%d - state != Task::State::PREPARING",
                         __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
            return;
        }
        task->state = Task::State::TRANSMITTING;
        task->lastActivatingTime = QDateTime::currentDateTime();
    }
    notifyTaskStateChanged(*task);

    if (task->starter == Task::Starter::SELF)
    {
        if (task->action == Task::Action::READ)
        {
            notifyTaskRemoteReadFileBlock(*task, BLOCK_SIZE);
        }
        else
        {
            successWriteRemoteFileBlock(taskId, task->progress);
        }
    }
}

void FileTransmission::TaskScheduler::remoteFailureOpenFile(const std::string& taskId, int errCode, const std::string& errString)
{
    TaskPrivatePtr task = getTask0(taskId);
    if (!task.get())
    {
        Logger::info("%s:%d - task %s does not exist",
                     __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
        return;
    }
    {
        std::lock_guard g(task->mutex);
        if (task->state != Task::State::PREPARING)
        {
            Logger::info("%s:%d - state != Task::State::PREPARING",
                         __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
            return;
        }
        task->state = Task::State::FAILED;
        task->error = errCode;
        task->reason = "[" + std::to_string(errCode) + "]" + errString;
        task->lastActivatingTime = QDateTime::currentDateTime();
    }
    notifyTaskStateChanged(*task);
}

std::pair<int, std::string> FileTransmission::TaskScheduler::writeLocalFileBlock(
        const std::string &taskId, unsigned long long progress, const char *data, size_t len)
{
    TaskPrivatePtr task = getTask0(taskId);
    if (!task.get())
        return {GLOBAL_PROTOCOL_ERR_OPERATE_FAILED, "The task does not exist"};

    return writeLocalFileBlock(*task, progress, data, len);
}

std::pair<int, std::string> FileTransmission::TaskScheduler::writeLocalFileBlock(
        TaskPrivate &task, unsigned long long progress, const char *data, size_t len)
{
    std::pair<int, std::string> ret;
    {
        std::lock_guard g(task.mutex);
        if (task.state != Task::State::TRANSMITTING)
            return {GLOBAL_PROTOCOL_ERR_OPERATE_FAILED, "The task is not in the transferring state"};

        if (!((task.starter == Task::Starter::SELF && task.action == Task::Action::READ)
                || (task.starter == Task::Starter::PARTNER && task.action == Task::Action::WRITE)))
            return {GLOBAL_PROTOCOL_ERR_OPERATE_FAILED, "Write operations cannot be performed on this task"};

        ret = task.write(progress, data, len);
        if (ret.first)
        {
            task.state = Task::State::FAILED;
            task.error = ret.first;
            task.reason = ret.second;
            task.closeIO();
        }
        task.lastActivatingTime = QDateTime::currentDateTime();
    }

    notifyTaskProgressChanged(task);
    if (ret.first)
    {
        notifyTaskStateChanged(task);
        if (task.starter == Task::Starter::SELF)
            notifyTaskRemoteCloseFile(task);
    }

    return ret;
}

std::pair<int, std::string> FileTransmission::TaskScheduler::readLocalFileBlock(
        const std::string &taskId, unsigned long long progress,  char* dst, size_t& len)
{
    TaskPrivatePtr task = getTask0(taskId);
    if (!task.get())
        return {GLOBAL_PROTOCOL_ERR_OPERATE_FAILED, "The task does not exist"};

    return readLocalFileBlock(*task, progress, dst, len);
}

std::pair<int, std::string> FileTransmission::TaskScheduler::readLocalFileBlock(
        TaskPrivate &task, unsigned long long progress,  char* dst, size_t& len)
{
    std::pair<int, std::string> ret;
    {
        std::lock_guard g(task.mutex);
        if (task.state != Task::State::TRANSMITTING)
            return {GLOBAL_PROTOCOL_ERR_OPERATE_FAILED, "The task is not in the transferring state"};

        if (!((task.starter == Task::Starter::SELF && task.action == Task::Action::WRITE)
                || (task.starter == Task::Starter::PARTNER && task.action == Task::Action::READ)))
            return {GLOBAL_PROTOCOL_ERR_OPERATE_FAILED, "Read operations cannot be performed on this task"};

        ret = task.read(progress, dst, len);
        if (ret.first)
        {
            if (task.starter == Task::Starter::SELF)
            {
                if (ret.first == GLOBAL_PROTOCOL_ERR_END_OF_FILE)
                {
                    task.state = Task::State::FINISHED;
                    task.finishingTime = QDateTime::currentDateTime();
                }
                else
                {
                    task.state = Task::State::FAILED;
                    task.error = ret.first;
                    task.reason = ret.second;
                }
            }
            task.closeIO();
        }
        task.lastActivatingTime = QDateTime::currentDateTime();
    }

    if (ret.first)
    {
        if (task.starter == Task::Starter::SELF)
        {
            notifyTaskStateChanged(task);
            notifyTaskRemoteCloseFile(task);
        }
    }

    return ret;
}

void FileTransmission::TaskScheduler::failureReadRemoteFileBlock(const std::string& taskId, int errCode, const std::string& errString)
{
    TaskPrivatePtr task = getTask0(taskId);
    if (!task.get())
    {
        Logger::info("%s:%d - task %s does not exist",
                     __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
        return;
    }
    {
        std::lock_guard g(task->mutex);
        if (task->starter != Task::Starter::SELF || task->state != Task::State::TRANSMITTING)
        {
            Logger::info("%s:%d - starter != Task::Starter::SELF || state != Task::State::TRANSMITTING",
                         __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
            return;
        }

        if (errCode == GLOBAL_PROTOCOL_ERR_END_OF_FILE)
        {
            task->state = Task::State::FINISHED;
            task->lastActivatingTime = task->finishingTime = QDateTime::currentDateTime();
        }
        else
        {
            task->state = Task::State::FAILED;
            task->error = errCode;
            task->reason = errString;
        }
        task->closeIO();
    }
    notifyTaskStateChanged(*task);
    notifyTaskRemoteCloseFile(*task);
}

void FileTransmission::TaskScheduler::successReadRemoteFileBlock(const std::string &taskId, unsigned long long progress, const char *data, size_t len)
{
    TaskPrivatePtr task = getTask0(taskId);
    if (!task.get())
    {
        Logger::info("%s:%d - task %s does not exist",
                     __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
        return;
    }


    auto err = writeLocalFileBlock(*task, progress, data, len);
    if (!err.first)
        notifyTaskRemoteReadFileBlock(*task, len);
}

void FileTransmission::TaskScheduler::failureWriteRemoteFileBlock(const std::string& taskId, int errCode, const std::string& errString)
{
    TaskPrivatePtr task = getTask0(taskId);
    if (!task.get())
    {
        Logger::info("%s:%d - task %s does not exist",
                     __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
        return;
    }
    {
        std::lock_guard g(task->mutex);
        if (task->starter != Task::Starter::SELF || task->state != Task::State::TRANSMITTING)
        {
            Logger::info("%s:%d - starter != Task::Starter::SELF || state != Task::State::TRANSMITTING",
                         __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
            return;
        }

        task->state = Task::State::FAILED;
        task->error = errCode;
        task->reason = errString;
        task->closeIO();
    }
    notifyTaskStateChanged(*task);
    notifyTaskRemoteCloseFile(*task);
}

void FileTransmission::TaskScheduler::successWriteRemoteFileBlock(const std::string &taskId, unsigned long long currentProgress)
{
    TaskPrivatePtr task = getTask0(taskId);
    if (!task.get())
    {
        Logger::info("%s:%d - task %s does not exist",
                     __PRETTY_FUNCTION__, __LINE__, taskId.c_str());
        return;
    }
    notifyTaskProgressChanged(*task);

    size_t len = BLOCK_SIZE;
    char* data = new char[len];
    auto err = readLocalFileBlock(*task, currentProgress, data, len);
    if (!err.first)
        notifyTaskRemoteWriteFileBlock(*task, currentProgress, data, len);
    delete[] data;
}

bool FileTransmission::TaskScheduler::getTask(const std::string &taskId, FileTransmission::Task& dst)
{
    TaskPrivatePtr task = getTask0(taskId);
    if (!task.get())
        return false;

    std::lock_guard g(task->mutex);
    dst = *task;
    return true;
}

FileTransmission::TaskScheduler::TaskScheduler()
{

}

FileTransmission::TaskScheduler::TaskPrivatePtr FileTransmission::TaskScheduler::getTask0(const std::string &taskId)
{
    TaskPrivatePtr ret;
    std::lock_guard guard(m_mutex);
    auto it = m_tasks.find(taskId);
    if (it != m_tasks.end())
        ret = it->second;
    return ret;
}

void FileTransmission::TaskScheduler::notifyTaskAdded(TaskPrivate &task)
{
    persistNewTask(task);

    std::scoped_lock guard(m_mutex, task.mutex);
    for (auto it : m_followers)
        it.second.taskAddedCallback(task);
}

void FileTransmission::TaskScheduler::notifyTaskRemoved(TaskPrivate &task)
{
    unpersistTask(task);

    std::scoped_lock guard(m_mutex, task.mutex);
    for (auto it : m_followers)
        it.second.taskRemovedCallback(task);
}

void FileTransmission::TaskScheduler::notifyTaskRemoteOpenFile(TaskPrivate &task)
{
    std::scoped_lock guard(m_mutex, task.mutex);
    for (auto it : m_followers)
        it.second.remoteOpenFileCallback(task);
}

void FileTransmission::TaskScheduler::notifyTaskRemoteCloseFile(TaskPrivate &task)
{
    std::scoped_lock guard(m_mutex, task.mutex);
    for (auto it : m_followers)
        it.second.remoteCloseFileCallback(task);
}

void FileTransmission::TaskScheduler::notifyTaskStateChanged(TaskPrivate &task)
{
    flushTaskProgress(task);

    std::scoped_lock guard(m_mutex, task.mutex);
    for (auto it : m_followers)
        it.second.stateChangedCallback(task);
}

void FileTransmission::TaskScheduler::notifyTaskProgressChanged(TaskPrivate &task)
{
    QDateTime now = QDateTime::currentDateTime();
    {
        std::lock_guard g(task.mutex);
        if (task.lastFlushProgressTime.addMSecs(PROGRESS_FLUSH_INTERVAL) > now)
            return;
        task.lastFlushProgressTime = now;
    }
    flushTaskProgress(task);

    std::scoped_lock guard(m_mutex, task.mutex);
    for (auto it : m_followers)
        it.second.progressChangedCallback(task);
}

void FileTransmission::TaskScheduler::notifyTaskRemoteWriteFileBlock(TaskPrivate& task, unsigned long long progress, const char* data, size_t len)
{
    std::scoped_lock guard(m_mutex, task.mutex);
    for (auto it : m_followers)
        it.second.remoteWriteFileBlockCallback(task, progress, data, len);
}

void FileTransmission::TaskScheduler::notifyTaskRemoteReadFileBlock(TaskPrivate& task, size_t len)
{
    std::scoped_lock guard(m_mutex, task.mutex);
    for (auto it : m_followers)
        it.second.remoteReadFileBlockCallback(task, task.progress, len);
}

void FileTransmission::TaskScheduler::exec()
{
    while (!m_exit)
    {
        QDateTime now = QDateTime::currentDateTime();
        TaskPrivatePtr task;
        {
            std::lock_guard guard(m_mutex);
            if (!m_queuedLocalTasks.empty())
            {
                task = m_queuedLocalTasks.front();
                m_queuedLocalTasks.pop_front();
            }
        }

        if (task.get())
        {
            task->mutex.lock();
            switch(task->state)
            {
            case Task::State::WAITING_FOR_START:
                {
                    auto err = task->initIO(task->overrideFile);
                    if (err.first)
                    {
                        task->state = Task::State::FAILED;
                        task->error = err.first;
                        task->reason = err.second;
                    }
                    else
                        task->state = Task::State::PREPARING;
                    task->mutex.unlock();

                    notifyTaskStateChanged(*task);
                    if (!err.first)
                    {
                        notifyTaskRemoteOpenFile(*task);
                        std::lock_guard guard(m_mutex);
                        m_queuedLocalTasks.push_back(task);
                    }
                }
                break;
            case Task::State::PREPARING:
            case Task::State::TRANSMITTING:
                {
                    if (task->lastActivatingTime.addSecs(TIME_OUT) < now)
                    {
                        task->state = Task::State::STOPED;
                        task->mutex.unlock();
                        notifyTaskStateChanged(*task);
                    }
                    else
                        task->mutex.unlock();
                    std::lock_guard guard(m_mutex);
                    m_queuedLocalTasks.push_back(task);
                }
                break;
            case Task::State::STOPED:
            case Task::State::FINISHED:
            case Task::State::FAILED:
                {
                    task->mutex.unlock();

                    if (task->starter == Task::Starter::PARTNER)
                    {
                        std::lock_guard guard(m_mutex);
                        m_tasks.erase(task->id);
                    }
                }
                break;
            default:
                assert(false);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

bool FileTransmission::TaskScheduler::initDB()
{
    QDir dir;
    dir.mkpath(DB_DIR);

    m_db = QSqlDatabase::addDatabase("QSQLITE", "FileTransmission::TaskScheduler");
    m_db.setDatabaseName(DB_PATH);
    bool ret = m_db.open();
    if (!ret)
        Logger::error("%s:%d - open db failed", __PRETTY_FUNCTION__, __LINE__);
    return ret;
}

bool FileTransmission::TaskScheduler::initTable()
{
    QString sql = "CREATE TABLE IF NOT EXISTS " TABLE_TASK "("
                    " id VARCHAR(32) PRIMARY KEY, "
                    " server_host VARCHAR(32) NOT NULL, "
                    " server_port UNSIGNED INT NOT NULL, "
                    " partner_id BIGINT NOT NULL, "
                    " action INT NOT NULL, "
                    " from_directory VARCHAR(4096) NOT NULL, "
                    " to_directory VARCHAR(4096) NOT NULL, "
                    " file_name VARCHAR(256) NOT NULL, "
                    " file_size UNSIGNED BIGINT NOT NULL, "
                    " creating_time DATETIME NOT NULL, "
                    " progress UNSIGNED BIGINT NOT NULL,"
                    " state INT NOT NULL,"
                    " finishing_time DATETIME DEFAULT NULL, "
                    " reason VARCHAR(256) DEFAULT ''"
                ")";

    QSqlQuery q(m_db);
    if (!q.exec(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }

    sql = "CREATE INDEX  IF NOT EXISTS index1 ON " TABLE_TASK "(server_host, server_port, partner_id)";
    if (!q.exec(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }

    return true;
}


bool FileTransmission::TaskScheduler::loadTasks()
{
    QString sql = "SELECT "
            " id, partner_id, action, from_directory, to_directory, "
            " file_name, file_size, creating_time, progress, state, server_host, server_port, "
            " finishing_time, reason FROM " TABLE_TASK ;
    QSqlQuery q(m_db);
    if (!q.exec(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }

    while (q.next())
    {
        std::string id = q.value(0).toString().toStdString();
        User::ID partnerId = q.value(1).toLongLong();
        Task::Action action = (Task::Action)q.value(2).toInt();
        std::string fromDirectory = q.value(3).toString().toStdString();
        std::string toDirectory = q.value(4).toString().toStdString();
        std::string fileName = q.value(5).toString().toStdString();
        unsigned long long fileSize = q.value(6).toULongLong();
        QDateTime creatingTime = q.value(7).toDateTime();
        unsigned long long progress = q.value(8).toULongLong();
        Task::State state = (Task::State)q.value(9).toInt();
        std::string serverHost = q.value(10).toString().toStdString();
        unsigned short serverPort = q.value(11).toUInt();
        QDateTime finishingTime = q.value(12).toDateTime();
        std::string reason = q.value(13).toString().toStdString();

        if (state != Task::State::FINISHED && state != Task::State::FAILED)
            state = Task::State::STOPED;

        addTask(id, serverHost, serverPort, partnerId, Task::Starter::SELF, action, state,
                fromDirectory, toDirectory, fileName, progress, fileSize, reason, false);
    }
    return true;
}

void FileTransmission::TaskScheduler::flushTaskProgress(TaskPrivate &task)
{
    if (task.starter != Task::Starter::SELF)
        return;

    {
        std::lock_guard g(m_mutex);
        QString sql = "UPDATE " TABLE_TASK " SET progress = ?, state = ?, reason = ? ";
        if (task.state == Task::State::FINISHED)
            sql += ", finishing_time = ?";
        sql += " WHERE id = ?";

        QSqlQuery q(m_db);
        if (!q.prepare(sql))
        {
            Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
            return;
        }
        int i = 0;
        q.bindValue(i++, (unsigned long long)task.progress);
        q.bindValue(i++, (int)task.state);
        q.bindValue(i++, task.reason.c_str());
        if (task.state == Task::State::FINISHED)
            q.bindValue(i++, task.finishingTime);
        q.bindValue(i++, task.id.c_str());
        if (!q.exec())
        {
            Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
            return;
        }
    }
}

void FileTransmission::TaskScheduler::persistNewTask(TaskPrivate &task)
{
    if (task.starter != Task::Starter::SELF)
        return;

    std::scoped_lock guard(m_mutex, task.mutex);
        QString sql = "INSERT OR IGNORE INTO " TABLE_TASK "("
                " id, server_host, server_port, partner_id, action, from_directory, to_directory, "
                " file_name, file_size, creating_time, progress, state) "
                " VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    QSqlQuery q(m_db);
    if (!q.prepare(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return;
    }
    int i = 0;
    q.bindValue(i++, task.id.c_str());
    q.bindValue(i++, task.serverHost.c_str());
    q.bindValue(i++, task.serverPort);
    q.bindValue(i++, (long long)task.partnerId);
    q.bindValue(i++, (int)task.action);
    q.bindValue(i++, task.fromDirectory.c_str());
    q.bindValue(i++, task.toDirectory.c_str());
    q.bindValue(i++, task.fileName.c_str());
    q.bindValue(i++, (unsigned long long)task.fileSize);
    q.bindValue(i++, task.creatingTime);
    q.bindValue(i++, (long long)task.progress);
    q.bindValue(i++, (int)task.state);
    if (!q.exec())
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return;
    }
}

void FileTransmission::TaskScheduler::unpersistTask(TaskPrivate &task)
{
    if (task.starter != Task::Starter::SELF)
        return;

    std::lock_guard guard(m_mutex);
   QString sql = "DELETE FROM " TABLE_TASK " WHERE id = ?";
   QSqlQuery q(m_db);
   if (!q.prepare(sql))
   {
       Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
       return;
   }
   q.bindValue(0, task.id.c_str());
   if (!q.exec())
   {
       Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
       return;
   }
}


FileTransmission::TaskScheduler::TaskPrivate::~TaskPrivate()
{
    closeIO();
}

std::pair<int, std::string> FileTransmission::TaskScheduler::TaskPrivate::initIO(bool force)
{
    if (this->fp)
        return {GLOBAL_PROTOCOL_ERR_NO_ERROR, ""};

    std::string fileDirectory;
    std::string mode;

    if ((starter == Task::Starter::SELF && action == Task::Action::WRITE)
            || (starter == Task::Starter::PARTNER && action == Task::Action::READ) )
    {
        fileDirectory = fromDirectory;
        mode = "rb";
    }
    else
    {
        fileDirectory = toDirectory;
        if (this->progress)
            mode = "ab";
        else
            mode = "wb";
        if (!force && QFile::exists((fileDirectory + "/" + this->fileName).c_str()))
        {
            return {GLOBAL_PROTOCOL_ERR_FILE_ALREADY_EXISTS, Global::Protocol::formatError(GLOBAL_PROTOCOL_ERR_FILE_ALREADY_EXISTS)};
        }
    }

#ifdef Q_OS_WIN
    QString path = (fileDirectory + "/" + this->fileName).c_str();
    QString modeq = mode.c_str();
    _wfopen_s(&this->fp, path.toStdWString().c_str(), modeq.toStdWString().c_str());
#else
    this->fp = fopen((fileDirectory + "/" + this->fileName).c_str(), mode.c_str());
#endif
    if (this->fp == nullptr)
        return {GLOBAL_PROTOCOL_ERR_OPERATE_FAILED, "open file failed"};

    if (this->progress)
    {
        auto err = seek(this->progress);
        if (err.first)
            return err;
    }

    return {GLOBAL_PROTOCOL_ERR_NO_ERROR, ""};
}

void FileTransmission::TaskScheduler::TaskPrivate::closeIO()
{
    if (fp)
    {
        fclose(fp);
        fp = nullptr;
    }
}

std::pair<int, std::string> FileTransmission::TaskScheduler::TaskPrivate::write(unsigned long long seek, const char *data, size_t len)
{
    if (this->fp == nullptr)
    {
        return {GLOBAL_PROTOCOL_ERR_FILE_CLOSED, Global::Protocol::formatError(GLOBAL_PROTOCOL_ERR_FILE_CLOSED)};
    }

    if (seek != this->progress)
    {
        auto ret = this->seek(seek);
        if (ret.first)
            return ret;
        this->progress = seek;
    }

    size_t offset = 0;
    while (true)
    {
        size_t c = fwrite(data + offset, 1, len - offset, this->fp);
        if(!c)
        {
            fclose(this->fp);
            this->fp = nullptr;
            return  {GLOBAL_PROTOCOL_ERR_NO_ERROR, "write file failed"};
        }

        offset += c;
        assert(offset <= len);
        if (offset == len)
            break;
    }
    fflush(this->fp);
    this->progress += len;
    updateSpeed(QDateTime::currentMSecsSinceEpoch(), len);
    return {GLOBAL_PROTOCOL_ERR_NO_ERROR, ""};
}

std::pair<int, std::string> FileTransmission::TaskScheduler::TaskPrivate::read(unsigned long long seek,  char* dst, size_t& len)
{
    if (this->fp == nullptr)
    {
        return {GLOBAL_PROTOCOL_ERR_FILE_CLOSED, Global::Protocol::formatError(GLOBAL_PROTOCOL_ERR_FILE_CLOSED)};
    }

    if (seek != this->progress)
    {
        auto ret = this->seek(seek);
        if (ret.first)
            return ret;
        this->progress = seek;
    }

    len = fread(dst, 1, len, this->fp);
    if (!len)
    {
        if (feof(this->fp))
            return {GLOBAL_PROTOCOL_ERR_END_OF_FILE, Global::Protocol::formatError(GLOBAL_PROTOCOL_ERR_END_OF_FILE)};
        return {GLOBAL_PROTOCOL_ERR_OPERATE_FAILED, Global::Protocol::formatError(GLOBAL_PROTOCOL_ERR_OPERATE_FAILED)};
    }
    this->progress += len;
    updateSpeed(QDateTime::currentMSecsSinceEpoch(), len);
    return {GLOBAL_PROTOCOL_ERR_NO_ERROR, ""};
}

std::pair<int, std::string> FileTransmission::TaskScheduler::TaskPrivate::seek(unsigned long long pos)
{
#ifdef Q_OS_WIN
    std::fpos_t pos1 = pos;
#else
    std::fpos_t pos1;
    pos1.__pos = pos;
#endif
    if(fsetpos(fp, &pos1))
        return {GLOBAL_PROTOCOL_ERR_OPERATE_FAILED, "failed to set the file position indicator"};
    return {GLOBAL_PROTOCOL_ERR_NO_ERROR, ""};
}

void FileTransmission::TaskScheduler::TaskPrivate::updateSpeed(long long msec, size_t value)
{
    speedCalculation.update(msec, value);
    speed = speedCalculation.get();
}
