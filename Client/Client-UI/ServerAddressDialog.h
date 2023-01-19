#ifndef SERVERADDRESSDIALOG_H
#define SERVERADDRESSDIALOG_H

#include <QDialog>

namespace Ui {
class ServerAddressDialog;
}

class ServerAddressDialog : public QDialog
{
    Q_OBJECT

public:
    enum EditMode {ADD, MODIFY};

public:
    explicit ServerAddressDialog(QWidget *parent = nullptr);
    ~ServerAddressDialog();

    bool show(EditMode mode);

public:
    QString host;
    unsigned short port = 0;
    QString name;
    QString account;
    QString password;
    QString publicKey;

private slots:
    void on_pushButton_clicked();

private:
    Ui::ServerAddressDialog *ui;
    bool m_confirmed;
};

#endif // SERVERADDRESSDIALOG_H
