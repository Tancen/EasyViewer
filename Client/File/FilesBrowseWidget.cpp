#include "FilesBrowseWidget.h"
#include "ui_FilesBrowseWidget.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QAction>
#include "Global/Protocol/Protocol.h"
#include <QGraphicsColorizeEffect>
#include <QDebug>
#include "Global/Component/Logger/Logger.h"

#define ROLE_ENTRY  (Qt::UserRole + 1)

#define COLUMN_NAME  0
#define COLUMN_SIZE  1
#define COLUMN_MODIFYING_TIME  2

#define KEY_DIRECTORY   "KEY_DIRECTORY"
#define KEY_NAME        "KEY_NAME"
#define KEY_SIZE        "KEY_SIZE"
#define KEY_TYPE        "KEY_TYPE"
#define KEY_LAST_MODIFYING_TIME   "KEY_LAST_MODIFYING_TIME"

FilesBrowseWidget::FilesBrowseWidget(TransmitAction transmitAction, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilesBrowseWidget),
    m_skipIndexChangedEvent(0)
{
    ui->setupUi(this);

    if (transmitAction == UPLOAD)
    {
        QString styleSheet = "QPushButton { "
           "     border-image: url(:/Resource/send.png);"
           " }"

           " QPushButton::hover {"
           "     background-color: rgb(200, 200, 200);"
           " }"

           " QPushButton::pressed {"
           "     background-color: rgb(180, 180, 180);"
           " }";
        ui->btnTransmit->setToolTip("Upload");
        ui->btnTransmit->setStyleSheet(styleSheet);
        ui->layoutTools->removeWidget(ui->btnTransmit);
        ui->layoutTools->addStretch();
        ui->layoutTools->addWidget(ui->btnTransmit);
    }
    else
    {
        QString styleSheet = "QPushButton { "
           "     border-image: url(:/Resource/send2.png);"
           " }"

           " QPushButton::hover {"
           "     background-color: rgb(200, 200, 200);"
           " }"

           " QPushButton::pressed {"
           "     background-color: rgb(180, 180, 180);"
           " }";
        ui->btnTransmit->setToolTip("Download");
        ui->btnTransmit->setStyleSheet(styleSheet);
        ui->layoutTools->removeWidget(ui->btnTransmit);
        ui->layoutTools->insertStretch(0);
        ui->layoutTools->insertWidget(0, ui->btnTransmit);
    }
    ui->tblFileList->setColumnWidth(0, 240);
    ui->tblFileList->setColumnWidth(1, 100);
    ui->tblFileList->setColumnWidth(2, 180);

    QObject::connect(ui->tblFileList->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
        [this](int logicalIndex, Qt::SortOrder order)
        {
            ui->tblFileList->sortItems(logicalIndex, order);
        });

    QGraphicsColorizeEffect* effect;
    effect = new QGraphicsColorizeEffect();
    effect->setColor(QColor(3, 121, 220));
    ui->btnTransmit->setGraphicsEffect(effect);

    QAction *action;

    action = new QAction();
    action->setText("Transmit");
    action->setIcon(QIcon(":/Resource/loop.png"));
    QObject::connect(action, &QAction::triggered,
        [this](bool checked)
        {
            transmit();
        });
    ui->tblFileList->addAction(action);

    action = new QAction();
    action->setText("Refresh");
    action->setIcon(QIcon(":/Resource/refresh.png"));
    QObject::connect(action, &QAction::triggered,
        [this](bool checked)
        {
            listEntries(ui->edtDirectory->currentText().toStdString());
        });
    ui->tblFileList->addAction(action);

    action = new QAction();
    action->setText("Rename");
    action->setIcon(QIcon(":/Resource/rename.png"));
    QObject::connect(action, &QAction::triggered,
        [this](bool checked)
        {
            renameEntry();
        });
    ui->tblFileList->addAction(action);

    action = new QAction();
    action->setText("New Folder");
    action->setIcon(QIcon(":/Resource/newFolder.png"));
    QObject::connect(action, &QAction::triggered,
        [this](bool checked)
        {
            makeDirectory();
        });
    ui->tblFileList->addAction(action);

    action = new QAction();
    action->setText("Delete");
    action->setIcon(QIcon(":/Resource/delete.png"));
    QObject::connect(action, &QAction::triggered,
        [this](bool checked)
        {
            deleteEntry();
        });
    ui->tblFileList->addAction(action);

}

FilesBrowseWidget::~FilesBrowseWidget()
{
    delete ui;
}

void FilesBrowseWidget::setEntries(const QVector<FileTransmission::EntryInfo> &entries)
{
    ui->tblFileList->setRowCount(entries.size());
    for (size_t r = 0; r < entries.size(); r++)
    {
        const auto& entry = entries.at(r);
        setEntry(r, entry);
    }
}

void FilesBrowseWidget::setCurrentDirectory(const std::string &directory)
{
    m_skipIndexChangedEvent++;
    int r = addDirectory(directory);
    ui->edtDirectory->setCurrentIndex(r);
    m_skipIndexChangedEvent--;
}

int FilesBrowseWidget::addDirectory(const std::string &directory, int index)
{
    int ret = 0;
    m_skipIndexChangedEvent++;
    do
    {
        ret = ui->edtDirectory->findText(directory.c_str());
        if (ret >= 0)
            break;

        ui->edtDirectory->insertItem(index, directory.c_str());
        if (index < 0)
            ret = 0;
        else if (index >= ui->edtDirectory->count() - 1)
            ret = ui->edtDirectory->count() - 1;
        else
            ret = index;
    } while (false);
    m_skipIndexChangedEvent--;
    return ret;
}

std::string FilesBrowseWidget::getCurrentDirectory()
{
    return ui->edtDirectory->currentText().toStdString();
}

void FilesBrowseWidget::appendEntry(const FileTransmission::EntryInfo &entry)
{
    int r = ui->tblFileList->rowCount();
    bool exists = false;
    for (int i = 0; i < r; i++)
    {
        if (ui->tblFileList->item(i, COLUMN_NAME)->text().toStdString() == entry.name)
        {
            exists = true;
            r = i;
            break;
        }
    }

    if (!exists)
        ui->tblFileList->setRowCount(r + 1);

    setEntry(r, entry);
}

void FilesBrowseWidget::removeEntry(const FileTransmission::EntryInfo &entry)
{
    removeEntry(entry.name);
}

void FilesBrowseWidget::removeEntry(const std::string &name)
{
    for (int i = 0; i < ui->tblFileList->rowCount(); i++)
    {
        std::string name1 = ui->tblFileList->item(i, COLUMN_NAME)->text().toStdString();
        if (name1 == name)
        {
            ui->tblFileList->removeRow(i);
            return;
        }
    }
}

void FilesBrowseWidget::on_btnHome_clicked()
{
    goHome();
}

void FilesBrowseWidget::on_btnRefresh_clicked()
{
    listEntries(ui->edtDirectory->currentText().toStdString());
}

void FilesBrowseWidget::on_btnDelete_clicked()
{
    deleteEntry();
}

void FilesBrowseWidget::on_btnMakeDir_clicked()
{
    makeDirectory();
}

void FilesBrowseWidget::on_btnTransmit_clicked()
{
    transmit();
}

bool FilesBrowseWidget::getCurrentEntry(FileTransmission::EntryInfo &entry)
{
    int row = ui->tblFileList->currentRow();
    if (row == -1)
    {
        return false;
    }

    auto item = ui->tblFileList->item(row, COLUMN_NAME);
    QVariantMap m = item->data(ROLE_ENTRY).toMap();
    entry.directory = m[KEY_DIRECTORY].toString().toStdString();
    entry.name = m[KEY_NAME].toString().toStdString();
    entry.size = m[KEY_SIZE].toULongLong();
    entry.type = (FileTransmission::EntryType)m[KEY_TYPE].toInt();
    entry.lastModifyingTime = m[KEY_LAST_MODIFYING_TIME].toDateTime();
    return true;
}

void FilesBrowseWidget::goHome()
{
    if (onGoHome)
        onGoHome(this);
}

void FilesBrowseWidget::goUp()
{
    if (onGoUp)
        onGoUp(this);
}

void FilesBrowseWidget::listEntries(const std::string& directory)
{
    if (onListEntries)
        onListEntries(this, directory);
}

void FilesBrowseWidget::deleteEntry()
{
    FileTransmission::EntryInfo entry;
    if (!getCurrentEntry(entry))
    {
        QMessageBox::about(nullptr, "failed", "Not choose row");
        return;
    }

    if(QMessageBox::question(nullptr, "Delete file",
            QString("Are you sure that you want to delete '") + entry.name.c_str() + "' ?",
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        return;

    if (onDeleteEntry)
    {
        onDeleteEntry(this, entry);
    }
}

void FilesBrowseWidget::makeDirectory()
{
    bool isOk;
    std::string name = QInputDialog::getText(nullptr, "Make folder",
                        "Please input the folder name",
                        QLineEdit::Normal, QString(), &isOk).toStdString();
    if (!isOk)
        return;

    if (name.empty())
    {
        QMessageBox::about(nullptr, "", "Directory name invalid");
        return;
    }

    if (onMakeDirectory)
        onMakeDirectory(this, name);
}

void FilesBrowseWidget::transmit()
{
    FileTransmission::EntryInfo entry;
    if (!getCurrentEntry(entry))
    {
        QMessageBox::about(nullptr, "failed", "Not choose row");
        return;
    }

    if (entry.type != FileTransmission::EntryType::FILE)
    {
        QMessageBox::about(nullptr, "failed", "The entry is not a file");
        return;
    }

    if (onTransmit)
    {
        onTransmit(this, entry);
    }
}

void FilesBrowseWidget::renameEntry()
{
    FileTransmission::EntryInfo entry;
    if (!getCurrentEntry(entry))
    {
        QMessageBox::about(nullptr, "failed", "Not choose row");
        return;
    }

    bool isOk;
    std::string newName = QInputDialog::getText(nullptr, "Rename",
                        "Please input the new name",
                        QLineEdit::Normal, QString(), &isOk).toStdString();
    if (!isOk)
        return;

    if (newName.empty())
    {
        QMessageBox::about(nullptr, "", "name invalid");
        return;
    }

    if (onRenameEntry)
    {
        onRenameEntry(this, entry.name, newName);
    }
}

void FilesBrowseWidget::renameEntry(const std::string &oldName, const std::string &newName, const QDateTime& lastModifyingTime)
{
    for (int i = 0; i < ui->tblFileList->rowCount(); i++)
    {
        auto item = ui->tblFileList->item(i, COLUMN_NAME);
        if (item->text().toStdString() == oldName)
        {
            ui->tblFileList->item(i, COLUMN_MODIFYING_TIME)->setText(lastModifyingTime.toString(GLOBAL_PROTOCOL_DATETIME_FORMAT));
            item->setText(newName.c_str());
            QVariantMap m = item->data(ROLE_ENTRY).toMap();
            m[KEY_NAME] = newName.c_str();
            m[KEY_LAST_MODIFYING_TIME] = lastModifyingTime;
            item->setData(ROLE_ENTRY, m);
            return;
        }
    }
}

void FilesBrowseWidget::setEntry(int row, const FileTransmission::EntryInfo &entry)
{
    static QIcon ICON_FOLDER(":/Resource/folder.png");
    static QIcon ICON_FILE(":/Resource/file.png");

    QVariantMap m;
    m[KEY_DIRECTORY] = entry.directory.c_str();
    m[KEY_NAME] = entry.name.c_str();
    m[KEY_SIZE] = entry.size;
    m[KEY_TYPE] = (int)entry.type;
    m[KEY_LAST_MODIFYING_TIME] = entry.lastModifyingTime;
    auto item = new QTableWidgetItem(entry.name.c_str());
    if (entry.type == FileTransmission::EntryType::DIRECTORY)
    {
        item->setForeground(QBrush(QColor(0, 128, 0, 255)));
        item->setIcon(ICON_FOLDER);
    }
    else
    {
        item->setIcon(ICON_FILE);
    }
    item->setData(ROLE_ENTRY, m);
    ui->tblFileList->setItem(row, COLUMN_NAME, item);
    ui->tblFileList->setItem(row, COLUMN_SIZE,
            new FileSizeTableWidgetItem(entry.size));
    ui->tblFileList->setItem(row, COLUMN_MODIFYING_TIME,
            new DateTimeTableWidgetItem(entry.lastModifyingTime));
}

QString FilesBrowseWidget::formatSize(unsigned long long v)
{
    if (v == 0)
        return "-";

    static const double U = 1024 * 1024;
    if (v >= U)
        return QString::asprintf("%.2lf MB", v / U);
    return QString::asprintf("%.2lf KB", v / 1024.0);
}

void FilesBrowseWidget::on_tblFileList_cellDoubleClicked(int row, int column)
{
    FileTransmission::EntryInfo entry;
    if (!getCurrentEntry(entry))
    {
        QMessageBox::about(nullptr, "failed", "Not choose row");
        return;
    }

    if (entry.type == FileTransmission::EntryType::DIRECTORY)
    {
        listEntries(entry.directory + "/" + entry.name);
    }
}

void FilesBrowseWidget::on_btnUpperLevel_clicked()
{
    goUp();
}

FilesBrowseWidget::FileSizeTableWidgetItem::FileSizeTableWidgetItem(unsigned long long v)
    :   QTableWidgetItem(formatSize(v)),
        m_v(v)
{
    this->setTextAlignment(Qt::AlignCenter);
}

bool FilesBrowseWidget::FileSizeTableWidgetItem::operator<(const QTableWidgetItem &other) const
{
    const FileSizeTableWidgetItem* p = dynamic_cast<const FileSizeTableWidgetItem*>(&other);
    if (!p)
        return true;

    return this->m_v < p->m_v;
}

FilesBrowseWidget::DateTimeTableWidgetItem::DateTimeTableWidgetItem(const QDateTime &v)
    :   QTableWidgetItem(v.toString(GLOBAL_PROTOCOL_DATETIME_FORMAT)),
        m_v(v)
{
    this->setTextAlignment(Qt::AlignCenter);
}

bool FilesBrowseWidget::DateTimeTableWidgetItem::operator<(const QTableWidgetItem &other) const
{
    const DateTimeTableWidgetItem* p = dynamic_cast<const DateTimeTableWidgetItem*>(&other);
    if (!p)
        return true;

    return this->m_v < p->m_v;
}

void FilesBrowseWidget::on_edtDirectory_currentIndexChanged(int index)
{
    if (index >= 0 && !m_skipIndexChangedEvent)
    {
        listEntries(ui->edtDirectory->currentText().toStdString());
    }
}
