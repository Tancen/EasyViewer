#ifndef FILETRANSMISSION_H
#define FILETRANSMISSION_H

#include <string>
#include <QDateTime>
#include <vector>
#include <map>
#include <qsqldatabase.h>
#include <mutex>
#include <QDir>
#include <memory>
#include <thread>
#include <functional>
#include "Global/Define.h"
#include "Global/Component/SpeedCalculation/SpeedCalculation.h"

namespace FileTransmission
{
    enum class EntryType { DIRECTORY, FILE };

    struct EntryInfo
    {
        std::string directory;      //whatever the file type is,  the directory are without the file name
        std::string name;
        unsigned long long size;    //zero if the entry type is directory
        EntryType type;
        QDateTime lastModifyingTime;
    };

    void getEntries(const QDir &dir, QVector<FileTransmission::EntryInfo> &dst);

    struct Task
    {
        enum class State { WAITING_FOR_START, PREPARING,
                     TRANSMITTING, STOPED, FINISHED, FAILED  };

        enum class Action { WRITE, READ };
        enum class Starter { SELF, PARTNER };

        static const char* toString(State state);
        static const char* toString(Action action);

        std::string id;
        std::string serverHost;
        unsigned short serverPort;
        User::ID partnerId;
        Starter starter;
        Action action;
        std::string fromDirectory;
        std::string toDirectory;
        std::string fileName;
        State state;
        uint64_t progress;
        uint64_t fileSize;

        int error;
        std::string reason;

        double speed;
        QDateTime creatingTime;
        QDateTime finishingTime;

        bool overrideFile;
    };


    class TaskScheduler;
    typedef std::shared_ptr<TaskScheduler> TaskSchedulerPtr;

    class TaskScheduler
    {
        struct TaskPrivate : public Task
        {
            ::FILE* fp = nullptr;
            QDateTime lastActivatingTime;
            QDateTime lastFlushProgressTime;
            std::recursive_mutex mutex;
            SpeedCalculation speedCalculation;

            ~TaskPrivate();

            std::pair<int, std::string> initIO(bool force);
            void closeIO();

            std::pair<int, std::string> write(unsigned long long seek, const char *data, size_t len);
            std::pair<int, std::string> read(unsigned long long seek,  char* dst, size_t& len);
            std::pair<int, std::string> seek(unsigned long long pos);

            void updateSpeed(long long msec, size_t value);
        };
        typedef std::shared_ptr<TaskPrivate> TaskPrivatePtr;

        struct Callbacks
        {
            std::function<void(const Task& task)> taskAddedCallback;
            std::function<void(const Task& taskId)> taskRemovedCallback;
            std::function<void(const Task& task)> stateChangedCallback;
            std::function<void(const Task& task)> remoteOpenFileCallback;
            std::function<void(const Task& task)> remoteCloseFileCallback;
            std::function<void(const Task& task)> progressChangedCallback;
            std::function<void(const Task& task, unsigned long long progress, const char* data, size_t len)> remoteWriteFileBlockCallback;
            std::function<void(const Task& task, unsigned long long progress, size_t len)> remoteReadFileBlockCallback;
        };

    public:
        static TaskScheduler* share();
        static bool init();
        static void release();
        void exit();
        void waitingForExited();

        std::pair<int, std::string> addTaskActively(
                        const std::string& serverHost,
                        unsigned short serverPort,
                        User::ID partnerId,
                        Task::Action action,
                        const std::string& fromDirectory,
                        const std::string& toDirectory,
                        const std::string& fileName,
                        uint64_t progress,
                        uint64_t fileSize,
                        bool overrideFile = false
                     );
        std::pair<int, std::string> addTaskPassively(   const std::string& id,
                        const std::string& serverHost,
                        unsigned short serverPort,
                        User::ID partnerId,
                        Task::Action action,
                        const std::string& directory,
                        const std::string& fileName,
                        uint64_t progress,
                        uint64_t fileSize,
                        bool overrideFile = false
                     );

        bool subscribeEvents(void* object,
                             std::function<void(const Task& task)> taskAddedCallback,
                             std::function<void(const Task& taskId)> taskRemovedCallback,
                             std::function<void(const Task& task)> remoteOpenFileCallback,
                             std::function<void(const Task& task)> remoteCloseFileCallback,
                             std::function<void(const Task& task)> stateChangedCallback,
                             std::function<void(const Task& task)> progressChangedCallback,
                             std::function<void(const Task& task, unsigned long long progress, const char* data, size_t len)> remoteWriteFileBlockCallback,
                             std::function<void(const Task& task, unsigned long long progress, size_t len)> remoteReadFileBlockCallback);
        void unsubscribeEvents(void* object);

        void removeTask(const std::string& taskId);

        void stopTask(const std::string& taskId);
        void resumeTask(const std::string& taskId);

        void remoteSuccessOpenFile(const std::string& taskId);
        void remoteFailureOpenFile(const std::string& taskId, int errCode, const std::string& errString);

        std::pair<int, std::string> writeLocalFileBlock(const std::string& taskId, unsigned long long progress, const char* data, size_t len);
        std::pair<int, std::string> readLocalFileBlock(const std::string& taskId, unsigned long long progress, char* dst, size_t& len);

        void failureReadRemoteFileBlock(const std::string& taskId, int errCode, const std::string& errString);
        void successReadRemoteFileBlock(const std::string& taskId, unsigned long long progress, const char* data, size_t len);

        void failureWriteRemoteFileBlock(const std::string& taskId, int errCode, const std::string& errString);
        void successWriteRemoteFileBlock(const std::string& taskId, unsigned long long currentProgress);

        bool getTask(const std::string& taskId, Task& dst);

    private:
        TaskScheduler();

        std::pair<int, std::string> addTask(   const std::string& id,
                        const std::string& serverHost,
                        unsigned short serverPort,
                        User::ID partnerId,
                        Task::Starter starter,
                        Task::Action action,
                        Task::State state,
                        const std::string& fromDirectory,
                        const std::string& toDirectory,
                        const std::string& fileName,
                        uint64_t progress,
                        uint64_t fileSize,
                        const std::string& reason,
                        bool overrideFile
                     );


        TaskPrivatePtr getTask0(const std::string& taskId);

        std::pair<int, std::string> writeLocalFileBlock(TaskPrivate& task, unsigned long long progress, const char* data, size_t len);
        std::pair<int, std::string> readLocalFileBlock(TaskPrivate& task, unsigned long long progress, char* dst, size_t& len);

        void notifyTaskAdded(TaskPrivate& task);
        void notifyTaskRemoved(TaskPrivate& task);
        void notifyTaskRemoteOpenFile(TaskPrivate& task);
        void notifyTaskRemoteCloseFile(TaskPrivate& task);
        void notifyTaskStateChanged(TaskPrivate& task);
        void notifyTaskProgressChanged(TaskPrivate& task);
        void notifyTaskRemoteWriteFileBlock(TaskPrivate& task, unsigned long long progress, const char* data, size_t len);
        void notifyTaskRemoteReadFileBlock(TaskPrivate& task, size_t len);

        void exec();

        bool initDB();
        bool initTable();
        bool loadTasks();
        void flushTaskProgress(TaskPrivate& task);
        void persistNewTask(TaskPrivate& task);
        void unpersistTask(TaskPrivate& task);

    private:
        QSqlDatabase m_db;

        std::map<std::string, TaskPrivatePtr> m_tasks;
        std::list<TaskPrivatePtr> m_queuedLocalTasks;

        std::map<void*, Callbacks> m_followers;

        std::recursive_mutex m_mutex;
        bool m_exit = false;
        std::thread m_thread;

        static TaskScheduler* s_this;
    };
}
#endif // FILETRANSMISSIONTASK_H
