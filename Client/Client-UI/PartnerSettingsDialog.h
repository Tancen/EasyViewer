#ifndef PARTNERSETTINGSDIALOG_H
#define PARTNERSETTINGSDIALOG_H

#include <QDialog>
#include "Global/Define.h"

namespace Ui {
class PartnerSettingsDialog;
}

class PartnerSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PartnerSettingsDialog(QWidget *parent = nullptr);
    ~PartnerSettingsDialog();

    bool show();

private slots:
    void on_btnConfirm_clicked();

public:
    User::ID partnerId;
    QString alias;
    QString authString;

private:
    Ui::PartnerSettingsDialog *ui;
    bool m_confirmed;
};

#endif // PARTNERSETTINGSDIALOG_H
