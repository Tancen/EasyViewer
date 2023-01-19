#include "ScreenViewer.h"
#include "ui_ScreenViewer.h"
#include <qimage.h>
#include <qapplication.h>
#include <QActionGroup>
#include <qmessagebox.h>
#include "Global/Protocol/Protocol.h"
#include "Global/Protocol/Common/Echo.pb.h"
#include "Global/Protocol/Screen/Screen.pb.h"
#include "Global/Protocol/Screen/Control.pb.h"
#include "Global/Protocol/Error.h"
#include "Global/Component/Logger/Logger.h"
#include <string.h>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QPushButton>
#include <QSplitter>
#include <QPixmap>
#include <zlib.h>
#include <zconf.h>
#include <QFile>
#include "Clipboard.h"
#include "Shortcuts.h"
#include "UsualParseProtobufDataMacro.h"
#include <QDateTime>
#include <QPainter>
#include <libyuv.h>
#include "Define.h"

#define SCALE_MODE_UNKNOW       -1
#define SCALE_MODE_SCALED       1
#define SCALE_MODE_ONE_TO_ONE   0

#define DEFAULT_DISPLAYER_WIDTH     1366
#define DEFAULT_DISPLAYER_HEIGHT    768
#define MAIN_LAYOUT_MARGIN  5
#define MENU_BAR_HEIGHT 22
#define STATUS_BAR_HEIGHT 20
#define MAX_WIDTH 16777215
#define MAX_HEIGHT MAX_WIDTH

using namespace std::placeholders;

ScreenViewer::ScreenViewer(const std::string& peerName, const std::string& peerHost, const std::string& peerIP,
                           unsigned short peerPort,
                           const std::string& publicKey,
                           const std::string& account, const std::string& password,
                           User::ID partnerUserId, const std::string& partnerAlias,
                           const std::string& authString, QWidget* parent) :
    QMainWindow(parent),
    ViewerBase(peerHost, peerIP, peerPort, publicKey, account, password, partnerUserId, authString),
    m_scaledPanel(SCALE_MODE_UNKNOW),
    ui(new Ui::ScreenViewer)

{
    ui->setupUi(this);

    roseCurtain();

    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setWindowFlags(Qt::Window);
    setMouseTracking(true);

    QObject::connect(&m_timerTick, SIGNAL(timeout()), this, SLOT(tick()));
    QObject::connect(this, SIGNAL(signalError(int,QString)), this, SLOT(handleError(int,QString)));
    QObject::connect(this, SIGNAL(partnerResized()), this, SLOT(adjustDisplayerSize()));
    QObject::connect(this, SIGNAL(signalUpdateCurtainText(bool,bool,bool)),
                     this, SLOT(updateCurtainText(bool,bool,bool)));

    this->resize(DEFAULT_DISPLAYER_WIDTH + MAIN_LAYOUT_MARGIN * 2, DEFAULT_DISPLAYER_HEIGHT + MENU_BAR_HEIGHT + STATUS_BAR_HEIGHT);
    QString title = QString::asprintf("%s[%s]-%s[%lld]", peerName.c_str(), peerHost.c_str(), partnerAlias.c_str(), partnerUserId);
    this->setWindowTitle(title);

    initMenuBar();
    initPanel();
    initStatusBar();

    Clipboard::share()->subscribe(this, std::bind(&ScreenViewer::setPartnerClipboardText, this, std::placeholders::_1));
}

ScreenViewer::~ScreenViewer()
{
    Clipboard::share()->unsubscribe(this);
    m_client->disconnect();
    while (m_client->connected())
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

    delete ui;
}

int ScreenViewer::role()
{
    return GLOBAL_CONNECTION_ROLE_SCREEN_VISITOR;
}

std::pair<bool, std::string> ScreenViewer::init()
{
    auto ret = ViewerBase::init();
    if (ret.first)
    {
        m_timerTick.start(10);
    }
    return ret;
}

QImage ScreenViewer::screenShot()
{
    std::lock_guard g(m_mutex);
    if (m_displayer && m_curtainDowned)
    {
        return m_displayer->grab().toImage();
    }
    return QImage();
}

void ScreenViewer::initMenuBar()
{
    ui->menuDisplayMode->removeAction(ui->action1_1);
    ui->menuDisplayMode->removeAction(ui->actionAutoScale);

    QActionGroup* g = new QActionGroup(this);
    g->setExclusive(true);
    ui->menuDisplayMode->addAction(g->addAction(ui->action1_1));
    ui->menuDisplayMode->addAction(g->addAction(ui->actionAutoScale));
}

void ScreenViewer::initPanel()
{
    m_displayer = new DesktopDisplayer();
    m_displayer->resize(DEFAULT_DISPLAYER_WIDTH, DEFAULT_DISPLAYER_HEIGHT);
    m_displayer->onWheel = std::bind(&ScreenViewer::onPanelWheelEvent, this, _1);
    m_displayer->onMouseMove = std::bind(&ScreenViewer::onPanelMouseMoveEvent, this, _1, _2);
    m_displayer->onMouseDoubleClick = std::bind(&ScreenViewer::onPanelMouseDoubleClickEvent, this, _1, _2, _3);
    m_displayer->onMousePress = std::bind(&ScreenViewer::onPanelMousePressEvent, this, _1, _2, _3);
    m_displayer->onMouseRelease = std::bind(&ScreenViewer::onPanelMouseReleaseEvent, this, _1, _2, _3);
    m_displayer->onKeyPress = std::bind(&ScreenViewer::onPanelKeyPressEvent, this, _1);
    m_displayer->onKeyRelease = std::bind(&ScreenViewer::onPanelKeyReleaseEvent, this, _1);

    changePanelAutoScaleMode();
}

void ScreenViewer::initStatusBar()
{
    QSplitter * splitter = new QSplitter();
    splitter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    splitter->setMaximumWidth(5);
    ui->statusbar->addWidget(splitter);

    m_lblSpeed = new QLabel("0 KB/s");
    ui->statusbar->addWidget(m_lblSpeed);

    ui->statusbar->addWidget(new QLabel("|"));

    m_lblTimeDelay = new QLabel("time-delay unknow ms");
    ui->statusbar->addWidget(m_lblTimeDelay);
}

void ScreenViewer::onPanelWheelEvent(int angleDelta)
{
    Global::Protocol::Screen::ControlMouseWheel request;
    request.set_delta(angleDelta);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_WHEEL, request, data, m_aesDirect)
    send(data);
}

void ScreenViewer::onPanelMouseMoveEvent(int x, int y)
{
    if (x != m_xSelfMousePos || y != m_ySelfMousePos)
    {
        m_xSelfMousePos = x;
        m_ySelfMousePos = y;
        m_selfMousePosChanged = true;
        synchronizeMousePosition();
    }
}

void ScreenViewer::onPanelMouseDoubleClickEvent(Qt::MouseButton button, int x, int y)
{
    onPanelMousePressEvent(button, x, y);
}

void ScreenViewer::onPanelMousePressEvent(Qt::MouseButton button, int x, int y)
{
    Global::Protocol::Screen::ControlMouseButtonPress request;
    request.set_x(x);
    request.set_y(y);
    request.set_button(button);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_PRESS, request, data, m_aesDirect)
    send(data);
}

void ScreenViewer::onPanelMouseReleaseEvent(Qt::MouseButton button, int x, int y)
{
    Global::Protocol::Screen::ControlMouseButtonRelease request;
    request.set_x(x);
    request.set_y(y);
    request.set_button(button);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_BUTTON_RELEASE, request, data, m_aesDirect)
    send(data);
}

void ScreenViewer::onPanelKeyPressEvent(Qt::Key key)
{
    Global::Protocol::Screen::ControlKeyPress request;
    request.set_key(key);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_KEY_PRESS, request, data, m_aesDirect)
    send(data);
}

void ScreenViewer::onPanelKeyReleaseEvent(Qt::Key key)
{
    Global::Protocol::Screen::ControlKeyRelease request;
    request.set_key(key);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_KEY_RELEASE, request, data, m_aesDirect)
    send(data);
}

void ScreenViewer::whenLoggedIn()
{
    emit signalUpdateCurtainText(true, true, false);

    Global::Protocol::Screen::RequestSubscribeScreen request;
    request.set_user_id(getPartnerUserId());
    request.set_auth_string(getAuthString());

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_SUBSCRIBE_SCREEN, request, data, m_aes)
    send(data);
}

void ScreenViewer::whenLoginFailed(int errCode, const std::string &errString)
{
    emit signalError(errCode, errString.c_str());
}

void ScreenViewer::setPartnerClipboardText(const std::string &text)
{
    Global::Protocol::Screen::SetClipboardText request;
    request.set_text(text);

    EasyIO::ByteBuffer buf;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_SET_CLIPBOARD_TEXT, request, buf, m_aesDirect)
    send(buf);
}

void ScreenViewer::updateCurtainText(bool connected, bool loggedIn, bool authorized)
{
    std::string text;
    do
    {
        if (!connected)
        {
            text += "Connecting ..";
            break;
        }
        text += "Connected\n";

        if (!loggedIn)
        {
            text += "Logging in ..";
            break;
        }
        text += "Logged in\n";

        if (!authorized)
        {
            text += "Authorizing ..";
            break;
        }
        text += "Authorized\n";
        text += "Waiting for image ..\n";

    } while(false);

    std::lock_guard g(m_mutex);
    m_curtainText = text.c_str();
}

bool ScreenViewer::handleCompleteData(unsigned tag, const unsigned char *data, size_t len)
{
    switch (tag)
    {
    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_SUBSCRIBE_SCREEN:
        handleResponseOfSubscribeScreen(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_PUBLISH_SCREEN_FRAME:
        handlePublishedScreenFrame(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_PUBLISH_CURSOR_POSITION:
        handlePublishCursorPosition(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_SET_CLIPBOARD_TEXT:
        handleCommandOfSetClipboardText(data, len);
        return true;

    case GLOBAL_PROTOCOL_NETWORK_TAG_RESPONSE_ECHO:
        handleResponseOfEcho(data, len);
        return true;
    }
    return false;
}

void ScreenViewer::handleResponseOfSubscribeScreen(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::Screen::ResponseSubscribeScreen, response, data, len, aes())

    if (response.err_code() != GLOBAL_PROTOCOL_ERR_NO_ERROR)
    {
        emit signalError(response.err_code(), response.err_string().c_str());
    }
    else
    {
        std::string errorString;
        m_aesDirect = Cryptology::AES<EasyIO::ByteBuffer>::newInstance(response.secret_key(), &errorString);
        if (!m_aesDirect.get())
        {
            emit signalError(GLOBAL_PROTOCOL_ERR_UNKNOW, errorString.c_str());
            return;
        }

        emit signalUpdateCurtainText(true, true, true);
    }
}

void ScreenViewer::handlePublishedScreenFrame(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::Screen::PublishScreenFrame, publishes, data, len, m_aesDirect)

    long long t = QDateTime::currentMSecsSinceEpoch();
    m_speedCalculation.update(t, len);
    m_speed = m_speedCalculation.get();

    unsigned w = publishes.w(), h = publishes.h();

    if ((w != m_wPartner || h != m_hPartner) || !m_decoder.get())
    {
        m_mutex.lock();
        m_wPartner = w;
        m_hPartner = h;

        auto adjustSize = [](unsigned v)
        {
            unsigned n = v;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            n += 1;
            return (n >> 1) == v ? v : n;
        };

        unsigned textureWidth = adjustSize(m_wPartner);
        unsigned textureHeight = adjustSize(m_hPartner);

        m_decoder.reset();
        ISVCDecoder *decoder;
        int err = WelsCreateDecoder(&decoder);
        assert(!err);

        SDecodingParam sDecParam = {0};
        sDecParam.sVideoProperty.size = sizeof (sDecParam.sVideoProperty);
        sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
        sDecParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
        decoder->Initialize(&sDecParam);
        m_decoder.reset(decoder);

        EasyIO::ByteBuffer buf;
        buf.fill(0, textureWidth * textureHeight * 4);
        m_displayer->setScreenBuffer(buf, textureWidth, textureHeight, m_wPartner, m_hPartner);

        m_mutex.unlock();

        emit partnerResized();
    }

    unsigned char *planes[3] = {0};

    SBufferInfo bufInfo;
    memset(&bufInfo, 0, sizeof(SBufferInfo));
    bufInfo.uiInBsTimeStamp = publishes.seq();
    int err = m_decoder->DecodeFrameNoDelay((unsigned char*)publishes.data().c_str(), publishes.data().length(), planes, &bufInfo);
    if (err)
        return;

    if (bufInfo.iBufferStatus == 1)
    {
        m_displayer->beginFillFrame();
        EasyIO::ByteBuffer buf = m_displayer->getScreenBuffer();
        libyuv::I420ToABGR(
                planes[0], bufInfo.UsrData.sSystemBuffer.iStride[0],
                planes[1], bufInfo.UsrData.sSystemBuffer.iStride[1],
                planes[2], bufInfo.UsrData.sSystemBuffer.iStride[1],
                buf.uReadableBytes(), m_displayer->textureWidth() * 4, m_wPartner, m_hPartner
                );
        m_displayer->endFillFrame();
    }
}

void ScreenViewer::handlePublishCursorPosition(const unsigned char *data, size_t len)
{
    //do nothing
}

void ScreenViewer::handleCommandOfSetClipboardText(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::Screen::SetClipboardText, request, data, len, m_aesDirect)
            Clipboard::share()->setText(request.text());
}

void ScreenViewer::handleResponseOfEcho(const unsigned char *data, size_t len)
{
    USUAL_PARSE_PROTOBUF_DATA_MACRO(m_client, Global::Protocol::Common::ResponseEcho, response, data, len, aes())

    long long now = QDateTime::currentMSecsSinceEpoch();
    m_timeDelay = (now - response.time()) / 2;
}

void ScreenViewer::whenConnecting(EasyIO::TCP::IConnection *con)
{
    emit signalUpdateCurtainText(false, false, false);
}

void ScreenViewer::whenConnected(EasyIO::TCP::IConnection *con)
{
    emit signalUpdateCurtainText(true, false, false);
}

void ScreenViewer::whenConnectFailed(EasyIO::TCP::IConnection *con, const std::string &reason)
{
    emit signalError(GLOBAL_PROTOCOL_ERR_CONNECT_FAILED, reason.c_str());
}

void ScreenViewer::whenDisconnected(EasyIO::TCP::IConnection *con)
{
    emit signalError(GLOBAL_PROTOCOL_ERR_DISCONNECTED, "Disconnected");
}

void ScreenViewer::setPartnerMousePosition(int x, int y)
{
    Global::Protocol::Screen::ControlMouseMove request;
    request.set_x(x);
    request.set_y(y);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_CONTROL_MOUSE_MOVE, request, data, m_aesDirect)
    send(data);
}

bool ScreenViewer::changePanelAutoScaleMode()
{
    if (m_scaledPanel == SCALE_MODE_SCALED)
        return true;
    m_scaledPanel = SCALE_MODE_SCALED;

    m_panelLayout = new QVBoxLayout();
    m_panelLayout->setSpacing(0);
    m_panelLayout->setContentsMargins(0, 0, 0, 0);

    ui->panel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->panel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->panel->setMaximumSize(MAX_WIDTH, MAX_HEIGHT);
    this->setMaximumSize(MAX_WIDTH, MAX_HEIGHT);
    m_displayer->setParent(nullptr);
    ui->panel->setLayout(m_panelLayout);
    m_panelLayout->addWidget(m_displayer);

    m_displayer->changeDisplayMode(DesktopDisplayer::SCALED);
    return true;
}

bool ScreenViewer::changePanelOneToOneMode()
{
    if (m_scaledPanel == SCALE_MODE_ONE_TO_ONE)
        return true;

    if (!m_wPartner || !m_hPartner)
        return false;
    m_scaledPanel = SCALE_MODE_ONE_TO_ONE;

    delete m_panelLayout;
    m_panelLayout = nullptr;

    ui->panel->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->panel->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->panel->setLayout(nullptr);
    m_displayer->setParent(ui->panel);
    ui->panel->setWidget(m_displayer);

    adjustDisplayerSize();
    m_displayer->changeDisplayMode(DesktopDisplayer::ONE_TO_ONE);

    return true;
}

void ScreenViewer::synchronizeMousePosition()
{
    long long now = QDateTime::currentMSecsSinceEpoch();
    if (m_selfMousePosChanged && now - m_lastMouseSynchronizingTime >= 10)
    {
        setPartnerMousePosition(m_xSelfMousePos, m_ySelfMousePos);
        m_selfMousePosChanged = false;
        m_lastMouseSynchronizingTime = now;
    }
}

void ScreenViewer::roseCurtain()
{
    m_curtainDowned = false;
    ui->panel->hide();
}

void ScreenViewer::curtainDown()
{
    m_curtainDowned = true;
    ui->panel->show();
}

void ScreenViewer::printSpeed()
{
    static const double UNIT_MB = 1048576;
    static const double UNIT_KB = 1024;

    long long now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastPrintSpeedTime < 1000)
        return;

    m_lastPrintSpeedTime = now;
    long long speed = m_speed;

    QString str;
    if (speed >= UNIT_MB)
    {
        str = QString::number(speed / UNIT_MB, 'f', 1) + " MB/s";
    }
    else
    {
        str = QString::number(speed / UNIT_KB, 'f', 1) + " KB/s";
    }
    m_lblSpeed->setText(str);
}

void ScreenViewer::echo()
{
    long long now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastEchoTime < 5000)
        return;
    m_lastEchoTime = now;

    Global::Protocol::Common::RequestEcho request;
    request.set_time(now);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_REQUEST_ECHO, request, data, aes())
    send(data);
}

void ScreenViewer::printTimeDelay()
{
    long long now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastPrintTimeDelayTime < 5000)
        return;

    m_lastPrintTimeDelayTime = now;

    QString str = "time-delay ";
    if (m_timeDelay >= 0)
        str += QString::number(m_timeDelay);
    else
        str += "unknow";
    str += " ms";
    m_lblTimeDelay->setText(str);
}

void ScreenViewer::closeEvent(QCloseEvent *event)
{
    if (this->onClose)
        this->onClose(this);
    event->accept();
}

void ScreenViewer::paintEvent(QPaintEvent *event)
{
    if (!m_curtainDowned)
    {
        QPainter painter(this);
        painter.setPen(QColor(100, 100, 100));
        static const int W = 150, H = 100;
        int t = (this->height() - H) / 2;
        int l = (this->width() - W) / 2;
        painter.drawText(QRect(l, t, W, H), Qt::AlignTop | Qt::AlignLeft, m_curtainText);
        return;
    }

    QMainWindow::paintEvent(event);
}

void ScreenViewer::handleError(int errCode, QString errString)
{
    QMessageBox::critical(this, "error", QString::asprintf("err[%d]: %s", errCode, errString.toStdString().c_str()));
    this->close();
}

void ScreenViewer::adjustDisplayerSize()
{
    if (m_displayer && !m_scaledPanel)
    {
        const int R = 10, M = 2;
        m_displayer->resize(m_wPartner, m_hPartner);
        this->setMaximumSize(m_wPartner + MAIN_LAYOUT_MARGIN + R + M, m_hPartner + MENU_BAR_HEIGHT + STATUS_BAR_HEIGHT + R + M);
        ui->panel->setMaximumSize(m_wPartner + R + M, m_hPartner + R + M);
    }
    curtainDown();
}

void ScreenViewer::tick()
{
    synchronizeMousePosition();
    printSpeed();
    echo();
    printTimeDelay();
}

void ScreenViewer::on_action1_1_triggered()
{
    changePanelOneToOneMode();
}


void ScreenViewer::on_actionAutoScale_triggered()
{
    changePanelAutoScaleMode();
}

void ScreenViewer::on_actionPrintScreen_triggered()
{
    Global::Protocol::Screen::Shortcut request;
    request.set_key(Shortcut::SHORTCUT_PRINT_SCREEN);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_SHORTCUT, request, data, m_aesDirect)
    send(data);
}

void ScreenViewer::on_actionCtrlShiftEsc_triggered()
{
    Global::Protocol::Screen::Shortcut request;
    request.set_key(Shortcut::SHORTCUT_CTRL_SHIFT_ESC);

    EasyIO::ByteBuffer data;
    GLOBAL_PROTO_WRAPPER_DESC_WITH_ENCRYPT(GLOBAL_PROTOCOL_NETWORK_TAG_SHORTCUT, request, data, m_aesDirect)
    send(data);
}


void ScreenViewer::ISVCDecoderDeleter::operator()(ISVCDecoder *p)
{
    p->Uninitialize();
    WelsDestroyDecoder(p);
}
