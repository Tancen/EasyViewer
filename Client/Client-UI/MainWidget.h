#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QAction>
#include <QTimer>
#include "ServerAddressDialog.h"
#include "ViewerBase.h"
#include "Guard.h"
#include <QStyledItemDelegate>
#include <QImage>
#include <QSystemTrayIcon>


namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

private:
    class ItemDelegate : public QStyledItemDelegate
    {
    public:
        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    };

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

signals:
    void signalGuardConnectFailed();
    void signalGuardLoginFailed(int errCode, QString errString);
    void signalGuardLoggedIn();
    void signalGuardDisconnected();
    void signalError(int errorCode, QString errorString);

private slots:
    void on_btnConnect_clicked();
    void on_tabServerAddress_cellDoubleClicked(int row, int column);

    void onActionAddServerAddressTriggered();
    void onActionModifyServerAddressTriggered();
    void onActionRemoveServerAddressTriggered();
    void onActionPartnerSettingsTriggered();
    void onActionRemovePartnerTriggered();

    void onGuardConnectFailed();
    void onGuardLoginFailed(int errCode, QString errString);
    void onGuardLoggedIn();
    void onGuardDisconnected();
    void onError(int errorCode, QString errorString);

    void on_edtMyAuthString_editingFinished();

    void on_lstPartnerAddress_itemDoubleClicked(QListWidgetItem *item);

    void on_btnAuthCodeEye_clicked();

    void on_btnCopyThisNumber_clicked();

    void on_btnCopyAuthCode_clicked();

    void on_edtMyAuthString_returnPressed();

    void on_edtPartnerUserId_returnPressed();

    void on_edtPartnerAuthString_returnPressed();


    void tick();

private:
    bool eventFilter(QObject *obj, QEvent *event) override;

    void activateServerAddress(QTableWidgetItem* it);

    void savePartnerThumbnail(const std::string& serverHost, unsigned short serverPort, User::ID partnerUserId, QImage image);
    QImage loadPartnerThumbnail(const std::string& serverHost, unsigned short serverPort, User::ID partnerUserId);

    void addPartnerListWidgetItem(const std::string& serverHost, unsigned short serverPort, const std::string& partnerName,
                                  User::ID partnerUserId, const std::string& authString);
    void updatePartnerListWidgetIcon(const std::string& serverHost,  unsigned short serverPort, User::ID partnerUserId, QImage image);
    void signInPartner(const std::string& serverHost,  unsigned short serverPort, User::ID partnerUserId, const std::string& authString);
    bool isPartnerSignedIn(const std::string& serverHost,  unsigned short serverPort, User::ID partnerUserId);
    void adjustAuthString(const std::string& serverHost,  unsigned short serverPort, User::ID partnerUserId, const std::string& authString);

    void switchAuthCodeDisplayMode();
    void toShowAuthCodeMode();
    void toHideAuthCodeMode();
    bool inShowAuthCodeMode();

    void checkWorkingTerminals();
    void terminateAllTerminals();

    void closeEvent(QCloseEvent *event) override;
    void quit();

private:
    struct RemoteServer
    {
        std::string name;
        std::string ip;
        std::string host;
        unsigned short port;
        std::string account;
        std::string password;
        std::string publicKey;
        std::unique_ptr<Guard> guard;
    };

#ifdef Q_OS_WIN
    typedef HANDLE ProcessID;
#else
    typedef __pid_t ProcessID;
#endif


private:
    Ui::MainWidget *ui;
    QSystemTrayIcon *m_tray = nullptr;
    ServerAddressDialog m_serverAddressDialog;

    std::list<ProcessID> m_workingTerminals;

    std::unique_ptr<RemoteServer> m_currentRemoteServer = nullptr;
    int m_cooldown;
    QTimer m_timer;
};

#endif // MAINWIDGET_H
