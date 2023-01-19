#include "PartnerSettingsDialog.h"
#include "ui_PartnerSettingsDialog.h"
#include <qmessagebox.h>

PartnerSettingsDialog::PartnerSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PartnerSettingsDialog)
{
    ui->setupUi(this);
}

PartnerSettingsDialog::~PartnerSettingsDialog()
{
    delete ui;
}

bool PartnerSettingsDialog::show()
{
    m_confirmed = false;
    ui->edtId->setText(QString::number(partnerId));
    ui->edtAlias->setText(alias);
    ui->edtAuthCode->setText(authString);

    this->exec();
    return m_confirmed;
}

void PartnerSettingsDialog::on_btnConfirm_clicked()
{
    alias = ui->edtAlias->text();
    authString = ui->edtAuthCode->text();

    if (alias.isEmpty())
    {
        QMessageBox::about(this, "", "name invalid");
        return;
    }

    if (authString.isEmpty())
    {
        QMessageBox::about(this, "", "auth code invalid");
        return;
    }

    m_confirmed = true;
    this->close();
}

