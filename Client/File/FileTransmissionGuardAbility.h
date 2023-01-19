#ifndef FILETRANSMISSIONGUARDABILITY_H
#define FILETRANSMISSIONGUARDABILITY_H

#include "Guard.h"
#include <QDir>
#include "File/FileTransmission.h"
#include <QTimer>


class FileTransmissionGuardAbility  : public Guard::IAbility
{
public:
    FileTransmissionGuardAbility();
    ~FileTransmissionGuardAbility();

    int role() override;


private:
    bool handleCompleteData(unsigned tag, const unsigned char* data, size_t len ) override;

    void handleCommandOfVisitFiles(const unsigned char* data, size_t len);
    void handleCommandOfGoUp(const unsigned char* data, size_t len);
    void handleCommandOfGoHome(const unsigned char* data, size_t len);
    void handleCommandOfListEntries(const unsigned char* data, size_t len);
    void handleCommandOfDeleteEntry(const unsigned char* data, size_t len);
    void handleCommandOfMakeDirectory(const unsigned char* data, size_t len);
    void handleCommandOfRenameEntry(const unsigned char* data, size_t len);
    void handleCommandOfOpenFile(const unsigned char* data, size_t len);
    void handleCommandOfCloseFile(const unsigned char* data, size_t len);
    void handleCommandOfWriteFile(const unsigned char* data, size_t len);
    void handleCommandOfReadFile(const unsigned char* data, size_t len);

    void whenTaskAdded(const FileTransmission::Task& task);
    void whenTaskRemoved(const FileTransmission::Task& task);
    void whenTaskRemoteOpenFile(const FileTransmission::Task& task);
    void whenTaskRemoteCloseFile(const FileTransmission::Task& task);
    void whenTaskStateChanged(const FileTransmission::Task& task);
    void whenTaskProgressChanged(const FileTransmission::Task& task);
    void whenTaskRemoteWriteFileBlock(const FileTransmission::Task& task, unsigned long long progress, const char* data, size_t len);
    void whenTaskRemoteReadFileBlock(const FileTransmission::Task& task, unsigned long long progress, size_t len);

    void handleLoggedinEvent() override;
    void handleConnectionLostEvent() override;
};

#endif // FILETRANSMISSIONGUARDABILITY_H
