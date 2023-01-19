#ifndef FILETRANSMISSIONVIEWER_H
#define FILETRANSMISSIONVIEWER_H

#include <QWidget>
#include <QDir>
#include <QTimer>
#include <mutex>
#include <set>
#include "../ViewerBase.h"
#include "FilesBrowseWidget.h"
#include "File/FileTransmission.h"

namespace Ui {
class FileTransmissionViewer;
}

class FileTransmissionViewer : public QWidget, public ViewerBase
{
    Q_OBJECT

public:
    explicit FileTransmissionViewer(const std::string& peerName, const std::string& peerHost, const std::string& peerIP,
                                    unsigned short peerPort, const std::string& publicKey,
                                    const std::string& account, const std::string& password,
                                    User::ID partnerUserId, const std::string& partnerAlias, const std::string& authString,
                                    QWidget* parent = nullptr);
   ~FileTransmissionViewer();

   int role() override;

signals:
    void signalError(int errCode, QString errString, bool fatal);
    void signalPartnerDefaultDirectories(QVector<std::string> directories);
    void signalPartnerBrowserChangeCurrentPath(QString path);
    void signalPartnerBrowserLoadEntries(QVector<FileTransmission::EntryInfo> entries);
    void signalPartnerEntryDeleted(QString directory, QString name);
    void signalPartnerDirectoryMade(QString directory, QString name, QDateTime lastModifyingTime);
    void signalPartnerEntryRenamed(QString directory, QString oldName, QString newName, QDateTime lastModifyingTime);

    void signalTaskAdded(FileTransmission::Task task);
    void signalTaskRemoved(FileTransmission::Task task);
    void signalTaskStateChanged(FileTransmission::Task task);
    void signalTaskProgressChanged(QString taskId, uint64_t progress, uint64_t fileSize, double speed);
    void signalTaskOverwriteConsent(FileTransmission::Task task);

    void signalLoggedIn();
    void signalPartnerAuthenticated();

private:
    bool handleCompleteData(unsigned tag, const unsigned char* data, size_t len ) override;

    void handleResponseOfVisitFiles(const unsigned char* data, size_t len);
    void handleResponseOfGoUp(const unsigned char* data, size_t len);
    void handleResponseOfGoHome(const unsigned char* data, size_t len);
    void handleResponseOfListEntries(const unsigned char* data, size_t len);
    void handleResponseOfDeleteEntry(const unsigned char* data, size_t len);
    void handleResponseOfMakeDirectory(const unsigned char* data, size_t len);
    void handleResponseOfRenameEntry(const unsigned char* data, size_t len);

    void handleResponseOfOpenFile(const unsigned char* data, size_t len);
    void handleResponseOfWriteFileBlock(const unsigned char* data, size_t len);
    void handleResponseOfReadFileBlock(const unsigned char* data, size_t len);
    void handleCommandOfCloseFile(const unsigned char* data, size_t len);

    void whenConnectFailed(EasyIO::TCP::IConnection* con, const std::string& reason) override;
    void whenDisconnected(EasyIO::TCP::IConnection* con) override;
    void whenLoggedIn() override;
    void whenLoginFailed(int errCode, const std::string& errString) override;

    void whenSelfGoUp(FilesBrowseWidget* widget);
    void whenSelfGoHome(FilesBrowseWidget* widget);
    void whenSelfListEntries(FilesBrowseWidget* widget, const std::string& directory);
    void whenSelfDeleteEntry(FilesBrowseWidget* widget, const FileTransmission::EntryInfo& entry);
    void whenSelfMakeDirectory(FilesBrowseWidget*, const std::string& directory);
    void whenSelfRenameEntry(FilesBrowseWidget*, const std::string& oldName, const std::string& newName);
    void whenSelfTransmit(FilesBrowseWidget*, const FileTransmission::EntryInfo& entry);

    void whenPartnerGoUp(FilesBrowseWidget* widget);
    void whenPartnerGoHome(FilesBrowseWidget* widget);
    void whenPartnerEnterDirectory(FilesBrowseWidget* widget, const std::string& directory);
    void whenPartnerListEntries(FilesBrowseWidget* widget, const std::string& directory);
    void whenPartnerDeleteEntry(FilesBrowseWidget* widget, const FileTransmission::EntryInfo& entry);
    void whenPartnerMakeDirectory(FilesBrowseWidget*, const std::string& directory);
    void whenPartnerRenameEntry(FilesBrowseWidget*, const std::string& oldName, const std::string& newName);
    void whenPartnerTransmit(FilesBrowseWidget*, const FileTransmission::EntryInfo& entry);

    void whenTaskAdded(const FileTransmission::Task& task);
    void whenTaskRemoved(const FileTransmission::Task& task);
    void whenTaskRemoteOpenFile(const FileTransmission::Task& task);
    void whenTaskRemoteCloseFile(const FileTransmission::Task& task);
    void whenTaskStateChanged(const FileTransmission::Task& task);
    void whenTaskProgressChanged(const FileTransmission::Task& task);
    void whenTaskRemoteWriteFileBlock(const FileTransmission::Task& task, unsigned long long progress, const char* data, size_t len);
    void whenTaskRemoteReadFileBlock(const FileTransmission::Task& task, unsigned long long progress, size_t len);


    void addTask(const FileTransmission::EntryInfo& entry, FileTransmission::Task::Action action);

private slots:
    void handleError(int errCode, QString errString, bool fatal);
    void partnerBrowserAddHistories(QVector<std::string> directories);
    void partnerBrowserChangeCurrentPath(QString path);
    void partnerBrowserLoadEntries(QVector<FileTransmission::EntryInfo> entries);
    void partnerBrowserDeleteEntry(QString directory, QString name);
    void partnerBrowserRenameEntry(QString directory, QString oldName, QString newName, QDateTime lastModifyingTime);
    void partnerBrowserMakeDirectory(QString directory, QString name, QDateTime lastModifyingTime);

    void updateUITableForTaskAdded(FileTransmission::Task task);
    void updateUITableForTaskRemoved(FileTransmission::Task task);
    void updateUITableForTaskStateChanged(FileTransmission::Task task);
    void updateUITableForTaskProgressChanged(QString taskId, uint64_t progress, uint64_t fileSize, double speed);

    void updateUISelfId();
    void updateUIPartnerId();

    void stopSelectedTask();
    void resumeSelectedTask();
    void removeSelectedTask();
    void clearFinishedTasks();
    void showInExplorer();
    void fileOverwriteConsent(FileTransmission::Task task);

private:
    int64_t m_seq = 0;

    FilesBrowseWidget *m_selfBrowser;
    FilesBrowseWidget *m_partnerBrowser;

    QDir m_dirSelf;

    Ui::FileTransmissionViewer *ui;
};

#endif // FILETRANSMISSIONVIEWER_H
