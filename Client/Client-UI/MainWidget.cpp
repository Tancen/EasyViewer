#include <QtGlobal>
#ifdef Q_OS_WIN
    #include "Desktop/ScreenCapturer_win.h"
#else
    #include "Desktop/ScreenCapturer_linux.h"
    #include <unistd.h>
    #include <netdb.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif
#include "MainWidget.h"
#include "ui_MainWidget.h"
#include "PartnerSettingsDialog.h"
#include <qdebug.h>
#include "ServerAddressBook.h"
#include <QMessageBox>
#include <QClipboard>
#include <QPainter>
#include <qvariant.h>
#include "AuthChecker.h"
#include "Global/Component/Logger/Logger.h"
#include "Desktop/ScreenViewer.h"
#include "File/FileTransmissionViewer.h"
#include <QGraphicsColorizeEffect>
#include <QHostInfo>
#include <QMenu>
#include <QCloseEvent>
#include <QRegularExpressionValidator>
#include "Command/TerminalArguments.h"
#include "Command/TerminalGuardAbility.h"
#include "Desktop/ScreenGuardAbility.h"
#include "File/FileTransmissionGuardAbility.h"
#include "Define.h"

#define COLUMN_SERVER_ADDRESS   1
#define COLUMN_STATUS           0

#define ROLE_SERVER_ADDRESS     (Qt::UserRole + 0)
#define ROLE_ACTIVATING         (Qt::UserRole + 1)
#define ROLE_PARTNER_ADDRESS    (Qt::UserRole + 2)

#define KEY_NAME        "name"
#define KEY_HOST        "host"
#define KEY_IP          "ip"
#define KEY_PORT        "port"
#define KEY_ACCOUNT     "account"
#define KEY_PASSWORD    "password"
#define KEY_PUBLIC_KEY  "publicKey"
#define KEY_USER_ID     "userId"
#define KEY_AUTH_STRING "authString"

#define BACKGROUND_COLOR_LOGGING ("QLabel { border-radius: 8; background-color : rgb(240, 155, 80); }")
#define BACKGROUND_COLOR_LOGGED  ("QLabel { border-radius: 8; background-color : rgb(41, 112, 21); }")
#define BACKGROUND_COLOR_DISCONNECTED  ("QLabel { border-radius: 8; background-color : rgb(220, 0, 0); }")

#define PARTNER_THUMBNAIL_DIR (qApp->applicationDirPath() + "/data/thumbnails")
#define PARTNER_THUMBNAIL_WIDTH     170
#define PARTNER_THUMBNAIL_HEIGHT    100


const char* STYLESHEET_TRAY_MENU =
"        QMenu{                                       "
"            background-color: rgb(255, 255, 255);    "
"            border:none;                             "
"            margin: 10px 10px;                       "
"        }                                            "
"                                                     "
"        QMenu::item{                                 "
"            color:rgb(40,40,40);                     "
"            background-color:rgb(255, 255, 255);     "
"            padding: 2px 10px 2px 10px;              "
"            border-radius: 6px;                      "
"            height: 28px;                            "
"            border:none;                             "
"        }                                            "
"                                                     "
"        QMenu::item:selected{                        "
"            background-color:rgb(235,235,235);       "
"        }                                            ";

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget),
    m_serverAddressDialog(this),
    m_cooldown(0)
{
    ui->setupUi(this);
    QAction* action = nullptr;

    //tray
    m_tray = new QSystemTrayIcon(this);
    m_tray->setToolTip("EasyViewer");
    m_tray->setIcon(QIcon(ICON_32));

    QMenu* menu = new QMenu();
    menu->setStyleSheet(STYLESHEET_TRAY_MENU);
    action = new QAction();
    action->setText("Open Main Window");
    QObject::connect(action, &QAction::triggered, [this](bool checked)
    {
        this->showNormal();
    });
    menu->addAction(action);

    action = new QAction();
    action->setText("Quit");
    QObject::connect(action, &QAction::triggered, [this](bool checked)
    {
        quit();
    });
    menu->addAction(action);
    m_tray->setContextMenu(menu);

    QObject::connect(m_tray, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason)
    {
        if (reason == QSystemTrayIcon::DoubleClick)
            this->showNormal();
    });
    m_tray->show();

    // right
    ui->btnCopyThisNumber->installEventFilter(this);
    ui->btnAuthCodeEye->installEventFilter(this);
    ui->btnCopyAuthCode->installEventFilter(this);
    ui->edtMyAuthString->setText(AuthChecker::share()->getAuthString().c_str());
    ui->edtPartnerUserId->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]+")));
    toHideAuthCodeMode();

    QObject::connect(this, SIGNAL(signalGuardLoggedIn()),
                    this, SLOT(onGuardLoggedIn()));

    QObject::connect(this, SIGNAL(signalGuardDisconnected()),
                    this, SLOT(onGuardDisconnected()));

    QObject::connect(this, SIGNAL(signalGuardLoginFailed(int, QString)),
                    this, SLOT(onGuardLoginFailed(int, QString)));

    QObject::connect(this, SIGNAL(signalGuardConnectFailed()),
                    this, SLOT(onGuardConnectFailed()));

    QObject::connect(this, SIGNAL(signalError(int, QString)),
                    this, SLOT(onError(int, QString)));


    //left
    ui->tabServerAddress->setItemDelegate(new ItemDelegate());
    ui->tabServerAddress->setColumnWidth(COLUMN_STATUS, 16);

    std::list<ServerAddress> serverAddresses;
    bool b = ServerAddressBook::share()->listServerAddress(serverAddresses);
    if (!b)
    {
        QMessageBox::critical(this, "error", "load server address book failed");
    }

    menu = new QMenu(this);
    action = new QAction("Add");
    action->setIcon(QIcon(":/Resource/add.png"));
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onActionAddServerAddressTriggered()));
    menu->addAction(action);

    action = new QAction("Modify");
    action->setIcon(QIcon(":/Resource/modify.png"));
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onActionModifyServerAddressTriggered()));
    menu->addAction(action);

    action = new QAction("Remove");
    action->setIcon(QIcon(":/Resource/minus.png"));
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onActionRemoveServerAddressTriggered()));
    menu->addAction(action);

    ui->tabServerAddress->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->tabServerAddress, &QTableWidget::customContextMenuRequested, [this, menu](const QPoint &pos)
    {
        menu->exec(ui->tabServerAddress->mapToGlobal(pos));
    });

    action = new QAction("Settings");
    action->setIcon(QIcon(":/Resource/settings.png"));
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onActionPartnerSettingsTriggered()));
    ui->lstPartnerAddress->addAction(action);

    action = new QAction("Remove");
    action->setIcon(QIcon(":/Resource/minus.png"));
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onActionRemovePartnerTriggered()));
    ui->lstPartnerAddress->addAction(action);


    // ScreenCapturer
    if (!ScreenCapturer::init())
    {
        Logger::error("ScreenCapturer init failed");
    }

    QTableWidgetItem* activatedItem = nullptr;
    for (const auto & addr : serverAddresses)
    {
        QString name = addr.name.c_str();
        if (name.isEmpty())
        {
            name.append(addr.host.c_str()).append(":").append(QString::number(addr.port));
        }

        QTableWidgetItem *it1 = new QTableWidgetItem();
        it1->setText(name);
        QVariantMap data;
        data[KEY_NAME] = name;
        data[KEY_USER_ID] = "-";
        data[KEY_HOST] = addr.host.c_str();
        data[KEY_PORT] = addr.port;
        data[KEY_ACCOUNT] = addr.account.c_str();
        data[KEY_PASSWORD] = addr.password.c_str();
        data[KEY_PUBLIC_KEY] = addr.publicKey.c_str();
        it1->setData(ROLE_SERVER_ADDRESS, data);

        QTableWidgetItem *it2 = new QTableWidgetItem();
        it2->setData(ROLE_ACTIVATING, false);

        int r = ui->tabServerAddress->rowCount();
        ui->tabServerAddress->setRowCount(r + 1);

        ui->tabServerAddress->setItem(r, COLUMN_SERVER_ADDRESS, it1);
        ui->tabServerAddress->setItem(r, COLUMN_STATUS, it2);
        if (addr.activated)
            activatedItem = it1;
    }

    if (activatedItem)
        activateServerAddress(activatedItem);

    // tick
    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    m_timer.setInterval(1000);
    m_timer.start();
}

MainWidget::~MainWidget()
{
    terminateAllTerminals();
    m_currentRemoteServer.reset();
    delete ui;
}

void MainWidget::on_btnConnect_clicked()
{
    if (!m_currentRemoteServer.get() || !m_currentRemoteServer->guard.get() || !m_currentRemoteServer->guard->connected())
    {
        QMessageBox::about(this, "", "Not logged in!");
        return;
    }

    User::ID partnerUserId = ui->edtPartnerUserId->text().toLongLong();
    std::string authString = ui->edtPartnerAuthString->text().toStdString();

    if (!partnerUserId || partnerUserId < 0 || authString.empty())
    {
        QMessageBox::about(this, "", "Unknow partner address!");
        return;
    }

    if (m_currentRemoteServer.get() == nullptr)
    {
        QMessageBox::about(this, "", "Unknow server address!");
        return;
    }
    std::string serverName = m_currentRemoteServer->name;
    std::string serverHost = m_currentRemoteServer->host;
    std::string serverIP = m_currentRemoteServer->ip;
    unsigned serverPort = m_currentRemoteServer->port;
    std::string account =  m_currentRemoteServer->account;
    std::string password = m_currentRemoteServer->password;
    std::string publicKey = m_currentRemoteServer->publicKey;

    if (!isPartnerSignedIn(serverHost, serverPort, partnerUserId))
        signInPartner(serverHost, serverPort, partnerUserId, authString);
    else
        adjustAuthString(serverHost, serverPort, partnerUserId, authString);

    PartnerAddress partnerAddr;
    if (!ServerAddressBook::share()->getPartnerAddress(serverHost, serverPort, partnerUserId, partnerAddr))
    {
        QMessageBox::about(this, "", "Query partner info failed!");
        return;
    }

    if (ui->btnDesktop->isChecked())
    {
        ScreenViewer* viewer = new ScreenViewer(serverName, serverHost, serverIP, serverPort, publicKey, account, password,
                            partnerUserId, partnerAddr.alias, authString);
        viewer->onClose = [this](ScreenViewer* viewer)
        {
            QImage image = viewer->screenShot();
            if (!image.isNull())
            {
                image = image.scaled(PARTNER_THUMBNAIL_WIDTH, PARTNER_THUMBNAIL_HEIGHT);
                savePartnerThumbnail(viewer->peerHost(), viewer->peerPort(), viewer->getPartnerUserId(), image);
                updatePartnerListWidgetIcon(viewer->peerHost(), viewer->peerPort(), viewer->getPartnerUserId(), image);
            }
        };
        auto err = viewer->init();
        if (!err.first)
        {
            delete viewer;
            QMessageBox::critical(this, "error", err.second.c_str());
            return;
        }
        viewer->show();
        viewer->login();
    }
    else if (ui->btnTerminal->isChecked())
    {
        std::string path;
        std::string args;

        path = qApp->applicationDirPath().toStdString();
#ifdef Q_OS_LINUX
        path.append("/easyviewer-terminal-viewer");
//        args.append("gnome-terminal -- ");
#else
        path.append("/easyviewer-terminal-viewer.exe");
#endif
        args.append(path).append(" ");
        args.append("--").append(COMMAND_LINE_ARG_SERVER_HOST).append("=").append(serverHost).append(" ");
        args.append("--").append(COMMAND_LINE_ARG_SERVER_PORT).append("=").append(std::to_string(serverPort)).append(" ");
        args.append("--").append(COMMAND_LINE_ARG_ACCOUNT).append("=").append(account).append(" ");
        args.append("--").append(COMMAND_LINE_ARG_PASSWORD).append("=").append(password).append(" ");
        args.append("--").append(COMMAND_LINE_ARG_PARTNER_ID).append("=").append(std::to_string(partnerUserId)).append(" ");
        args.append("--").append(COMMAND_LINE_ARG_PARTNER_ALIAS).append("=\"").append(partnerAddr.alias).append("\" ");
        args.append("--").append(COMMAND_LINE_ARG_AUTH_STRING).append("=\"").append(authString).append("\" ");
        args.append("--").append(COMMAND_LINE_ARG_PUBLIC_KEY).append("=\"").append(publicKey).append("\" ");

        Logger::info(args.c_str());
        #ifdef Q_OS_LINUX
            __pid_t pid = fork();
            if (pid == -1)
            {
                QMessageBox::critical(nullptr, "", QString::asprintf("fork error[%d] : %s", errno, strerror(errno)));
            }
            else if (pid == 0)
            {
                int ret = execl("/usr/bin/uxterm",  "uxterm", "-cjk_width", "-sb", "-rightbar",
                            "-fa", "Monospace", "-bd", "black", "-bg", "black",
                            "-fg", "white", "-font", "10x20", "-geometry", "80x24+10+10",
                            "-e", args.c_str(), nullptr);
                if (ret == -1)
                {
                    Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, strerror(errno));
                }
                return;
            }
            else
                m_workingTerminals.push_back(pid);
        #else
            STARTUPINFOA si;
            memset(&si, 0, sizeof(si));
            si.wShowWindow = SW_SHOWNORMAL;
            si.cb = sizeof(si);

            PROCESS_INFORMATION pi;
            memset(&pi, 0, sizeof(pi));

            if(!CreateProcessA(path.c_str(), (char*)args.c_str(), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
            {
                QMessageBox::critical(nullptr, "", "create process failed");
                return;
            }
            m_workingTerminals.push_back(pi.hProcess);
        #endif
    }
    else if (ui->btnFile->isChecked())
    {
        FileTransmissionViewer* viewer = new FileTransmissionViewer(
                    serverName, serverHost, serverIP, serverPort, publicKey, account, password,
                    partnerUserId, partnerAddr.alias, authString);

        auto err = viewer->init();
        if (!err.first)
        {
            delete viewer;
            QMessageBox::critical(this, "error", err.second.c_str());
            return;
        }

        viewer->show();
        viewer->login();
    }
    else
        assert(false);

}

void MainWidget::on_tabServerAddress_cellDoubleClicked(int row, int column)
{
    QTableWidgetItem *it = ui->tabServerAddress->item(row, COLUMN_SERVER_ADDRESS);
    activateServerAddress(it);
}

void MainWidget::onActionAddServerAddressTriggered()
{
    if (m_serverAddressDialog.show(ServerAddressDialog::ADD))
    {
        if (ServerAddressBook::share()->addServerAddress(m_serverAddressDialog.name.toStdString(),
            m_serverAddressDialog.host.toStdString(), m_serverAddressDialog.port,
            m_serverAddressDialog.account.toStdString(), m_serverAddressDialog.password.toStdString(),
            m_serverAddressDialog.publicKey.toStdString()))
        {
            QString name = m_serverAddressDialog.name;
            if (name.isEmpty())
            {
                name.append(m_serverAddressDialog.host).append(":").append(QString::number(m_serverAddressDialog.port));
            }

            QTableWidgetItem *it1 = new QTableWidgetItem();
            it1->setText(name);
            QVariantMap data;
            data[KEY_NAME] = name;
            data[KEY_USER_ID] = "-";
            data[KEY_HOST] = m_serverAddressDialog.host;
            data[KEY_PORT] = m_serverAddressDialog.port;
            data[KEY_ACCOUNT] = m_serverAddressDialog.account;
            data[KEY_PASSWORD] = m_serverAddressDialog.password;
            data[KEY_PUBLIC_KEY] = m_serverAddressDialog.publicKey;
            it1->setData(ROLE_SERVER_ADDRESS, data);

            QTableWidgetItem *it2 = new QTableWidgetItem();
            it2->setData(ROLE_ACTIVATING, false);

            int r = ui->tabServerAddress->rowCount();
            ui->tabServerAddress->setRowCount(r + 1);
            ui->tabServerAddress->setItem(r, COLUMN_SERVER_ADDRESS, it1);
            ui->tabServerAddress->setItem(r, COLUMN_STATUS, it2);
        }
        else
        {
            QMessageBox::about(this, "", "Add server address failed");
        }
    }
}

void MainWidget::onActionModifyServerAddressTriggered()
{
    QTableWidgetItem *it = ui->tabServerAddress->currentItem();
    if (it == nullptr)
    {
        QMessageBox::about(this, "", "Please select an item!");
        return;
    }
    QVariantMap data = it->data(ROLE_SERVER_ADDRESS).toMap();
    m_serverAddressDialog.name = it->text();
    m_serverAddressDialog.host = data[KEY_HOST].toString();
    m_serverAddressDialog.port = data[KEY_PORT].toUInt();
    m_serverAddressDialog.account = data[KEY_ACCOUNT].toString();
    m_serverAddressDialog.password = data[KEY_PASSWORD].toString();
    m_serverAddressDialog.publicKey = data[KEY_PUBLIC_KEY].toString();

    if (m_serverAddressDialog.show(ServerAddressDialog::MODIFY))
    {
        if (ServerAddressBook::share()->updateServerAddress(m_serverAddressDialog.name.toStdString(),
            m_serverAddressDialog.host.toStdString(), m_serverAddressDialog.port,
            m_serverAddressDialog.account.toStdString(), m_serverAddressDialog.password.toStdString(),
            m_serverAddressDialog.publicKey.toStdString()))
        {
            QString name = m_serverAddressDialog.name;
            if (name.isEmpty())
            {
                name.append(m_serverAddressDialog.host).append(":").append(QString::number(m_serverAddressDialog.port));
            }

            it->setText(name);

            data[KEY_NAME] = name;
            data[KEY_USER_ID] = "-";
            data[KEY_HOST] = m_serverAddressDialog.host;
            data[KEY_PORT] = m_serverAddressDialog.port;
            data[KEY_ACCOUNT] = m_serverAddressDialog.account;
            data[KEY_PASSWORD] = m_serverAddressDialog.password;
            data[KEY_PUBLIC_KEY] = m_serverAddressDialog.publicKey;
            it->setData(ROLE_SERVER_ADDRESS, data);

            if (m_currentRemoteServer && m_currentRemoteServer->name == name.toStdString())
            {
                activateServerAddress(it);
            }
        }
        else
        {
            QMessageBox::about(this, "", "Modify server address failed");
        }
    }
}

void MainWidget::onActionRemoveServerAddressTriggered()
{
    QTableWidgetItem *it = ui->tabServerAddress->currentItem();
    if (it == nullptr)
    {
        QMessageBox::about(this, "", "Please select an item!");
        return;
    }

    QVariantMap data = it->data(ROLE_SERVER_ADDRESS).toMap();
    if (!ServerAddressBook::share()->removeServerAddress(data[KEY_HOST].toString().toStdString(), data[KEY_PORT].toUInt()))
    {
        QMessageBox::about(this, "", "Modify server address failed");
        return;
    }

    ui->tabServerAddress->removeRow(ui->tabServerAddress->currentRow());
}

void MainWidget::onActionPartnerSettingsTriggered()
{
    QListWidgetItem *it = ui->lstPartnerAddress->currentItem();
    if (it == nullptr)
    {
        QMessageBox::about(this, "", "Please select an item!");
        return;
    }

    PartnerSettingsDialog dialog;

    QVariantMap addr = it->data(ROLE_PARTNER_ADDRESS).toMap();
    dialog.partnerId = addr[KEY_USER_ID].toLongLong();
    dialog.authString = addr[KEY_AUTH_STRING].toString();
    dialog.alias = it->text();

    if (dialog.show())
    {
        if (!m_currentRemoteServer.get())
        {
            QMessageBox::about(this, "", "No server active!");
            return;
        }

        if(!ServerAddressBook::share()->updatePartnerAddress(m_currentRemoteServer->host,
                                                         m_currentRemoteServer->port,
                                                         dialog.alias.toStdString(),
                                                         dialog.partnerId,
                                                         dialog.authString.toStdString()))
        {
            QMessageBox::about(this, "", "update partner address failed!");
            return;
        }

        addr[KEY_AUTH_STRING] = dialog.authString;
        it->setData(ROLE_PARTNER_ADDRESS, addr);
        it->setText(dialog.alias);
    }
}

void MainWidget::onActionRemovePartnerTriggered()
{
    QListWidgetItem *it = ui->lstPartnerAddress->currentItem();
    if (it == nullptr)
    {
        QMessageBox::about(this, "", "Please select an item!");
        return;
    }

    QVariantMap data = it->data(ROLE_PARTNER_ADDRESS).toMap();
    if(!ServerAddressBook::share()->removePartnerAddress(m_currentRemoteServer->host,
                                                     m_currentRemoteServer->port,
                                                     data[KEY_USER_ID].toLongLong()))
    {
        QMessageBox::about(this, "", "remove partner address failed!");
        return;
    }
    delete it;
}

void MainWidget::onGuardConnectFailed()
{
    this->ui->lblConnectionState->setStyleSheet((BACKGROUND_COLOR_DISCONNECTED));
    this->ui->lblConnectionStateHint->setText("Connect failed");
}

void MainWidget::onGuardLoginFailed(int errCode, QString errString)
{
    this->ui->lblConnectionState->setStyleSheet((BACKGROUND_COLOR_DISCONNECTED));
    this->ui->lblConnectionState->setToolTip(errString);
    this->ui->lblConnectionStateHint->setText("Login failed");
}

void MainWidget::onGuardLoggedIn()
{
    if (m_currentRemoteServer.get())
    {
        ui->lblMyUserId->setText(QString::number(m_currentRemoteServer->guard->getUserId()));

        this->ui->lblConnectionState->setStyleSheet((BACKGROUND_COLOR_LOGGED));
        this->ui->lblConnectionStateHint->setText("Logged in to the server");
    }
}

void MainWidget::onGuardDisconnected()
{
    this->ui->lblConnectionState->setStyleSheet((BACKGROUND_COLOR_DISCONNECTED));
    this->ui->lblConnectionStateHint->setText("Not connected");
}


void MainWidget::onError(int errorCode, QString errorString)
{
    if (m_currentRemoteServer.get())
        QMessageBox::critical(this, "error", QString::asprintf("error[%d]: %s", errorCode, errorString.toStdString().c_str()));
}

void MainWidget::on_edtMyAuthString_editingFinished()
{
    AuthChecker::share()->setAuthString(ui->edtMyAuthString->text().toStdString());
}

void MainWidget::tick()
{
    const int MAX_COOLDOWN = 30;
    if (m_currentRemoteServer.get())
    {
        if (!m_currentRemoteServer->guard->connected())
        {
            if (m_cooldown <= 0)
            {
                m_cooldown = MAX_COOLDOWN;

                this->ui->lblConnectionState->setStyleSheet((BACKGROUND_COLOR_LOGGING));
                this->ui->lblConnectionState->setToolTip("");
                this->ui->lblConnectionStateHint->setText("Connecting to server");
                m_currentRemoteServer->guard->connect();
            }
            else if (!m_currentRemoteServer->guard->connecting())
            {
                this->ui->lblConnectionStateHint->setText("Reconnect after " + QString::number(m_cooldown) + " seconds");
                m_cooldown--;
            }

            return;
        }
        if (!m_currentRemoteServer->guard->loggedIn())
        {
            if (!m_currentRemoteServer->guard->loggingIn())
            {
                this->ui->lblConnectionStateHint->setText("Logging in");
                m_currentRemoteServer->guard->login();
            }
            return;
        }
    }

    checkWorkingTerminals();
}

void MainWidget::addPartnerListWidgetItem(const std::string &serverHost, unsigned short serverPort,
                                          const std::string &partnerName, User::ID partnerUserId, const std::string &authString)
{
    QPixmap map(PARTNER_THUMBNAIL_WIDTH, PARTNER_THUMBNAIL_HEIGHT);
    QPainter painter(&map);
    QImage image = loadPartnerThumbnail(serverHost, serverPort, partnerUserId);
    if (!image.isNull())
    {
        painter.drawImage(0, 0, image);
    }
    else
    {
        painter.setPen(Qt::NoPen);
        painter.fillRect(map.rect(), QColor(128, 128, 128));
    }

    QVariantMap data;
    data[KEY_HOST] = serverHost.c_str();
    data[KEY_PORT] = serverPort;
    data[KEY_USER_ID] = (long long)partnerUserId;
    data[KEY_AUTH_STRING] = authString.c_str();

    QListWidgetItem* it = new QListWidgetItem();
    it->setIcon(map);
    it->setText(partnerName.c_str());
    it->setData(ROLE_PARTNER_ADDRESS, data);
    ui->lstPartnerAddress->addItem(it);
}

void MainWidget::updatePartnerListWidgetIcon(const std::string &serverHost, unsigned short serverPort, User::ID partnerUserId, QImage image)
{
    for (int i = 0; i < ui->lstPartnerAddress->count(); i++)
    {
        QListWidgetItem* it = ui->lstPartnerAddress->item(i);
        QVariantMap data = it->data(ROLE_PARTNER_ADDRESS).toMap();
        if (data[KEY_HOST].toString().toStdString() == serverHost
                && data[KEY_PORT].toUInt() == serverPort
                && data[KEY_USER_ID].toLongLong() == partnerUserId)
        {
            QImage image = loadPartnerThumbnail(serverHost, serverPort, partnerUserId);
            if (!image.isNull())
            {
                QPixmap map(PARTNER_THUMBNAIL_WIDTH, PARTNER_THUMBNAIL_HEIGHT);
                QPainter painter(&map);
                painter.drawImage(0, 0, image);
                it->setIcon(map);
            }
        }
    }
}

void MainWidget::signInPartner(const std::string& serverHost,  unsigned short serverPort, User::ID partnerUserId, const std::string& authString)
{
    std::string partnerName = std::to_string(partnerUserId);
    if(ServerAddressBook::share()->addPartnerAddress(serverHost, serverPort, partnerName, partnerUserId, authString))
    {
        addPartnerListWidgetItem(serverHost, serverPort, partnerName, partnerUserId, authString);
    }
    else
    {
        QMessageBox::critical(nullptr, "", "sign in partner failed");
    }
}

bool MainWidget::isPartnerSignedIn(const std::string &serverHost, unsigned short serverPort, User::ID partnerUserId)
{
    for (int i = 0; i < ui->lstPartnerAddress->count(); i++)
    {
        auto it = ui->lstPartnerAddress->item(i);
        QVariantMap data = it->data(ROLE_PARTNER_ADDRESS).toMap();
        if (data.find(KEY_HOST).value().toString().toStdString() == serverHost
                && data.find(KEY_PORT)->toUInt() == serverPort
                && data.find(KEY_USER_ID)->toULongLong() == partnerUserId)
            return true;
    }
    return false;
}

void MainWidget::adjustAuthString(const std::string &serverHost, unsigned short serverPort, User::ID partnerUserId, const std::string &authString)
{
    for (int i = 0; i < ui->lstPartnerAddress->count(); i++)
    {
        auto it = ui->lstPartnerAddress->item(i);
        QVariantMap data = it->data(ROLE_PARTNER_ADDRESS).toMap();
        if (data.find(KEY_HOST).value().toString().toStdString() == serverHost
                && data.find(KEY_PORT)->toUInt() == serverPort
                && data.find(KEY_USER_ID)->toULongLong() == partnerUserId)
        {
            std::string oldAuthString = data.find(KEY_AUTH_STRING)->toString().toStdString();
            if (oldAuthString != authString)
            {
                ServerAddressBook::share()->updatePartnerAddress(serverHost, serverPort, it->text().toStdString(), partnerUserId, authString);
                data[KEY_AUTH_STRING] = authString.c_str();
                it->setData(ROLE_PARTNER_ADDRESS, data);
            }
        }
    }
}

void MainWidget::switchAuthCodeDisplayMode()
{
    if (inShowAuthCodeMode())
        toHideAuthCodeMode();
    else
        toShowAuthCodeMode();
}

void MainWidget::toShowAuthCodeMode()
{
    ui->edtMyAuthString->setEchoMode(QLineEdit::EchoMode::Normal);
    ui->edtMyAuthString->setEnabled(true);
    ui->btnAuthCodeEye->setStyleSheet("QPushButton { border-image: url(:/Resource/openEye.png);}");
    ui->btnAuthCodeEye->setToolTip("hide auth code");
}

void MainWidget::toHideAuthCodeMode()
{
    ui->edtMyAuthString->setEchoMode(QLineEdit::EchoMode::Password);
    ui->edtMyAuthString->setEnabled(false);
    ui->btnAuthCodeEye->setStyleSheet("QPushButton { border-image: url(:/Resource/closeEye.png);}");
    ui->btnAuthCodeEye->setToolTip("show auth code");
}

bool MainWidget::inShowAuthCodeMode()
{
    return ui->edtMyAuthString->echoMode() == QLineEdit::EchoMode::Normal;
}

void MainWidget::checkWorkingTerminals()
{
    for (auto it = m_workingTerminals.begin(); it != m_workingTerminals.end();)
    {
#ifdef Q_OS_WIN
        auto ret = WaitForSingleObject(*it, 0);
        if (ret == WAIT_OBJECT_0)
#else
        ProcessID id = *it;
        int status;
        auto ret = waitpid(id, &status, WNOHANG);
        if (ret == -1 || (ret == id && WIFEXITED(status)))
#endif
        {
            it = m_workingTerminals.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void MainWidget::terminateAllTerminals()
{
    while (!m_workingTerminals.empty())
    {
        ProcessID id = m_workingTerminals.front();
#ifdef Q_OS_WIN
        TerminateProcess(id, 1);
#else
        kill(id, SIGKILL);
#endif
        m_workingTerminals.pop_front();
    }
}

void MainWidget::closeEvent(QCloseEvent *event)
{
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        this->hide();
        event->ignore();
    }
    else
    {
        quit();
    }
}

void MainWidget::quit()
{
    if(QMessageBox::question(this, "Quit", "Are you sure to quit?",
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::StandardButton::Yes)
    {
        qApp->exit();
    }
}

void MainWidget::on_lstPartnerAddress_itemDoubleClicked(QListWidgetItem *item)
{
    QVariantMap data = item->data(ROLE_PARTNER_ADDRESS).toMap();
    ui->edtPartnerUserId->setText(data[KEY_USER_ID].toString());
    ui->edtPartnerAuthString->setText(data[KEY_AUTH_STRING].toString());
}


void MainWidget::on_btnCopyThisNumber_clicked()
{
    if (!ui->lblMyUserId->text().isEmpty())
        qApp->clipboard()->setText(ui->lblMyUserId->text());
}


void MainWidget::on_btnAuthCodeEye_clicked()
{
    switchAuthCodeDisplayMode();
}

bool MainWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->btnCopyThisNumber || obj == ui->btnAuthCodeEye || obj == ui->btnCopyAuthCode)
    {
        QPushButton *btn = dynamic_cast<QPushButton*>(obj);
        assert(btn);
        if (event->type() == QEvent::HoverEnter)
        {
            QGraphicsColorizeEffect *e = new QGraphicsColorizeEffect(nullptr);
            e->setColor(QColor(2, 180, 245));
            e->setStrength(1);
            btn->setGraphicsEffect(e);
            return true;
        }
        if (event->type() == QEvent::HoverLeave)
        {
            btn->setGraphicsEffect(nullptr);
            return true;
        }
    }

    return false;
}

void MainWidget::activateServerAddress(QTableWidgetItem *it)
{
    assert(it);
    QVariantMap serverAddr = it->data(ROLE_SERVER_ADDRESS).toMap();

    std::string name = serverAddr[KEY_NAME].toString().toStdString();
    std::string host = serverAddr[KEY_HOST].toString().toStdString();
    unsigned short port = serverAddr[KEY_PORT].toUInt();
    std::string account = serverAddr[KEY_ACCOUNT].toString().toStdString();
    std::string password = serverAddr[KEY_PASSWORD].toString().toStdString();
    std::string publicKey = serverAddr[KEY_PUBLIC_KEY].toString().toStdString();

    for (int i = 0; i < ui->tabServerAddress->rowCount(); i++)
    {
        auto *it = ui->tabServerAddress->item(i, COLUMN_STATUS);
        it->setData(ROLE_ACTIVATING, false);
    }

    hostent* hostInfo = gethostbyname(host.c_str());
    if (!hostInfo || !hostInfo->h_addr_list[0])
    {
        QMessageBox::critical(nullptr, "", "server host invalid");
        return;
    }
    std::string ip = inet_ntoa(*(struct in_addr*)hostInfo->h_addr_list[0]);
    serverAddr[KEY_IP] = ip.c_str();   //put ip
    it->setData(ROLE_SERVER_ADDRESS, serverAddr);

    ui->lblMyUserId->setText(serverAddr[KEY_USER_ID].toString());
    ui->lblServerName->setText(it->text());
    ui->lblServerHost->setText(host.c_str());

    m_currentRemoteServer.reset();
    RemoteServer* server = nullptr;
    server = new RemoteServer();
    server->name = name;
    server->ip = ip;
    server->host = host;
    server->port = port;
    server->account = account;
    server->password = password;
    server->publicKey = publicKey;
    server->guard.reset(new Guard(host, ip, port, publicKey, account, password));

    std::unique_ptr<TerminalGuardAbility> terminalGuardAbility(new TerminalGuardAbility());
    server->guard->addAbility(std::move(terminalGuardAbility));

    std::unique_ptr<ScreenGuardAbility> screenGuardAbility(new ScreenGuardAbility());
    server->guard->addAbility(std::move(screenGuardAbility));

    std::unique_ptr<FileTransmissionGuardAbility> fileTransmissionGuardAbility(new FileTransmissionGuardAbility());
    server->guard->addAbility(std::move(fileTransmissionGuardAbility));

    server->guard->onConnectFailed = [this](Guard* g)
    {
        emit signalGuardConnectFailed();
    };

    server->guard->onLoginFailed = [this](Guard* g, int errCode, const std::string& errString)
    {
        emit signalGuardLoginFailed(errCode, errString.c_str());
        g->disconnect();
    };

    server->guard->onLoggedIn = [this](Guard* g)
    {
        emit signalGuardLoggedIn();
    };

    server->guard->onDisconnected = [this](Guard* g)
    {
        emit signalGuardDisconnected();
    };

    server->guard->onError = [this](Guard* g, int errorCode, const std::string& errorString)
    {
        emit signalError(errorCode, errorString.c_str());
    };

    auto err = server->guard->init();
    if (!err.first)
    {
        QMessageBox::critical(this, "error", err.second.c_str());
        delete server;
        return;
    }

    m_cooldown = 0;
    m_currentRemoteServer.reset(server);

    std::list<PartnerAddress> addresses;
    bool b = ServerAddressBook::share()->listPartnerAddress(serverAddr[KEY_HOST].toString().toStdString(),
                            serverAddr[KEY_PORT].toUInt(), addresses);
    if (!b)
    {
        return;
    }

    ui->lstPartnerAddress->clear();
    for (const auto & addr : addresses)
    {
        QString name = addr.alias.c_str();
        if (name.isEmpty())
            name = QString::number(addr.userId);

        addPartnerListWidgetItem(host, port, name.toStdString(), addr.userId, addr.authString);
    }

    it = ui->tabServerAddress->item(it->row(), COLUMN_STATUS);
    it->setData(ROLE_ACTIVATING, true);
    ServerAddressBook::share()->activateServerAddress(host, port);
}

void MainWidget::savePartnerThumbnail(const std::string &serverHost, unsigned short serverPort,  User::ID partnerUserId, QImage image)
{
    QString dirPath = PARTNER_THUMBNAIL_DIR + "/" + serverHost.c_str() + "-" + QString::number(serverPort);
    QDir dir;
    dir.mkpath(dirPath);

    QString filePath = dirPath + "/" + QString::number(partnerUserId) + ".jpg";
    image.save(filePath);
}

QImage MainWidget::loadPartnerThumbnail(const std::string &serverHost, unsigned short serverPort,  User::ID partnerUserId)
{
    QString dirPath = PARTNER_THUMBNAIL_DIR + "/" + serverHost.c_str() + "-" + QString::number(serverPort);
    QString filePath = dirPath + "/" + QString::number(partnerUserId) + ".jpg";
    QImage image(filePath);
    return image;
}


void MainWidget::ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QColor fontColor;
    QColor backgroundColor;

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (option.state & QStyle::State_Selected)
    {
        fontColor = QColor(240, 240, 240);
        backgroundColor = QColor(0, 0, 0, 50);
    }
    else
    {
        fontColor = QColor(235, 235, 235);
        backgroundColor = QColor(0, 0, 0, 0);
    }

    painter->fillRect(option.rect, backgroundColor);


    if (index.column() == COLUMN_STATUS)
    {
        if (index.data(ROLE_ACTIVATING).toBool())
        {
            fontColor = QColor(255, 255, 255);
            QPen pen(fontColor);
            pen.setWidth(2);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            QPoint center = option.rect.center();
            painter->drawEllipse(center.x() - 4, center.y() - 4, 12, 12);
        }
        else
        {
            painter->fillRect(option.rect, QColor(0, 0, 0, 0));
        }
    }

    QString text = "  " + index.data(Qt::DisplayRole).toString();
    if (!text.isEmpty())
    {
        painter->setPen(fontColor);
        painter->drawText(option.rect, text, QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

    }
}

void MainWidget::on_btnCopyAuthCode_clicked()
{
    if (!ui->edtMyAuthString->text().isEmpty())
        qApp->clipboard()->setText(ui->edtMyAuthString->text());
}


void MainWidget::on_edtMyAuthString_returnPressed()
{
    toHideAuthCodeMode();
    focusNextChild();
}


void MainWidget::on_edtPartnerUserId_returnPressed()
{
    ui->edtPartnerAuthString->setFocus();
}


void MainWidget::on_edtPartnerAuthString_returnPressed()
{
    focusNextChild();
}

