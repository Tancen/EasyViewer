#include "ServerAddressDialog.h"
#include "ui_ServerAddressDialog.h"
#include <qmessagebox.h>
#include <QRegularExpressionValidator>

ServerAddressDialog::ServerAddressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerAddressDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->edtName->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9.a-zA-Z\-_ ]+")));
    ui->edtHost->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9.a-zA-Z\-]+")));
    ui->edtAccount->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9.a-zA-Z\-_@]+")));
    ui->edtPort->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]+")));
}

ServerAddressDialog::~ServerAddressDialog()
{
    delete ui;
}

bool ServerAddressDialog::show(ServerAddressDialog::EditMode mode)
{
    m_confirmed = false;
    ui->edtHost->setText(host);
    ui->edtPort->setText(QString::number(port));
    ui->edtName->setText(name);
    ui->edtAccount->setText(account);
    ui->edtPassword->setText(password);
    ui->edtPublicKey->setText(publicKey);

    if (mode == ADD)
    {
        this->setWindowTitle("Add server address");
        ui->edtHost->setEnabled(true);
        ui->edtPort->setEnabled(true);
    }
    else
    {
        this->setWindowTitle("Modify server address");
        ui->edtHost->setEnabled(false);
        ui->edtPort->setEnabled(false);
    }

    QDialog::exec();
    return m_confirmed;
}

void ServerAddressDialog::on_pushButton_clicked()
{
    host = ui->edtHost->text();

    bool isOk;
    unsigned p = ui->edtPort->text().toUInt(&isOk);
    if (!p || !isOk || p >= 65535)
    {
        QMessageBox::about(this, "", "Port invalid");
        return;
    }
    port = p;

    name = ui->edtName->text();
    account = ui->edtAccount->text();
    if (account.isEmpty())
    {
        QMessageBox::about(this, "", "Account invalid");
        return;
    }

    password = ui->edtPassword->text();
    if (password.isEmpty())
    {
        QMessageBox::about(this, "", "Password invalid");
        return;
    }

    publicKey = ui->edtPublicKey->toPlainText();
    if (publicKey.isEmpty())
    {
        QMessageBox::about(this, "", "Public Key invalid");
        return;
    }
    if (publicKey.length() > 1024)
    {
        QMessageBox::about(this, "", "Public Key too long(max 1024)");
        return;
    }

    m_confirmed = true;
    this->close();
}
