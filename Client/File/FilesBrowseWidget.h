#ifndef FILESBROWSEWIDGET_H
#define FILESBROWSEWIDGET_H

#include <QWidget>
#include <functional>
#include "File/FileTransmission.h"
#include <QTableWidgetItem>
#include <set>

namespace Ui {
class FilesBrowseWidget;
}

class FilesBrowseWidget : public QWidget
{
    Q_OBJECT

private:
    typedef QTableWidgetItem StringTableWidgetItem;

    class FileSizeTableWidgetItem : public QTableWidgetItem
    {
    public:
        FileSizeTableWidgetItem(unsigned long long v);

        bool operator< (const QTableWidgetItem & other) const;

    private:
        unsigned long long m_v;
    };

    class DateTimeTableWidgetItem : public QTableWidgetItem
    {
    public:
        DateTimeTableWidgetItem(const QDateTime& v);

        bool operator< (const QTableWidgetItem & other) const;

    private:
        QDateTime m_v;
    };

public:
    enum TransmitAction { UPLOAD, DOWNLOAD };

    explicit FilesBrowseWidget(TransmitAction transmitAction, QWidget *parent = nullptr);
    ~FilesBrowseWidget();

    void setEntries(const QVector<FileTransmission::EntryInfo>& entries);
    void setCurrentDirectory(const std::string& directory);
    int addDirectory(const std::string& directory, int index = 0);
    std::string getCurrentDirectory();
    void appendEntry(const FileTransmission::EntryInfo& entry);
    void removeEntry(const FileTransmission::EntryInfo& entry);
    void removeEntry(const std::string& name);
    void renameEntry(const std::string& oldName, const std::string& newName, const QDateTime& lastModifyingTime);

    void setEntry(int row, const FileTransmission::EntryInfo& entry);
    static QString formatSize(unsigned long long v);

private slots:

    void on_btnHome_clicked();

    void on_btnRefresh_clicked();

    void on_btnDelete_clicked();

    void on_btnMakeDir_clicked();

    void on_btnTransmit_clicked();

    void on_tblFileList_cellDoubleClicked(int row, int column);

    void on_btnUpperLevel_clicked();

    void on_edtDirectory_currentIndexChanged(int index);

private:
    bool getCurrentEntry(FileTransmission::EntryInfo& entry);

    void goHome();
    void goUp();
    void listEntries(const std::string& directory);
    void deleteEntry();
    void makeDirectory();
    void transmit();
    void renameEntry();

public:
    std::function<void(FilesBrowseWidget*, const std::string& oldName, const std::string& newName)> onRenameEntry;
    std::function<void(FilesBrowseWidget*)> onGoUp;
    std::function<void(FilesBrowseWidget*)> onGoHome;
    std::function<void(FilesBrowseWidget*, const std::string& directory)> onListEntries;
    std::function<void(FilesBrowseWidget*, const FileTransmission::EntryInfo& entry)> onDeleteEntry;
    std::function<void(FilesBrowseWidget*, const std::string& directory)> onMakeDirectory;
    std::function<void(FilesBrowseWidget*, const FileTransmission::EntryInfo& entry)> onTransmit;


private:
    Ui::FilesBrowseWidget *ui;
    int m_skipIndexChangedEvent;
};

#endif // FILEBROWSEWIDGET_H
