#ifndef SCREENVIEWER_H
#define SCREENVIEWER_H

#include <QMainWindow>
#include <mutex>
#include "../ViewerBase.h"
#include "DesktopDisplayer.h"
#include <QTimer>
#include <QScrollArea>
#include <QImage>
#include <QLabel>
#include <QDateTime>
#include "Global/Component/SpeedCalculation/SpeedCalculation.h"

#include "wels/codec_api.h"

namespace Ui {
class ScreenViewer;
}

class ScreenViewer : public QMainWindow, public ViewerBase
{
    Q_OBJECT

    class ISVCDecoderDeleter
    {
    public:
        void operator() (ISVCDecoder* p);
    };
public:
    explicit ScreenViewer(const std::string& peerName, const std::string& peerHost, const std::string& peerIP,
                          unsigned short peerPort, const std::string& publicKey,
                          const std::string& account, const std::string& password,
                          User::ID partnerUserId, const std::string& partnerAlias,
                          const std::string& authString, QWidget *parent = nullptr);
    ~ScreenViewer();

    int role() override;

    std::pair<bool, std::string> init() override;

    QImage screenShot();

signals:
    void signalError(int errCode, QString errString);
    void signalUpdateCurtainText(bool connected, bool loggedIn, bool authorized);
    void partnerResized();

private:
    void initMenuBar();
    void initPanel();
    void initStatusBar();

    void onPanelWheelEvent(int angleDelta);
    void onPanelMouseMoveEvent(int x, int y);
    void onPanelMouseDoubleClickEvent(Qt::MouseButton button, int x, int y);
    void onPanelMousePressEvent(Qt::MouseButton button, int x, int y);
    void onPanelMouseReleaseEvent(Qt::MouseButton button, int x, int y);
    void onPanelKeyPressEvent(Qt::Key key);
    void onPanelKeyReleaseEvent(Qt::Key key);

    bool handleCompleteData(unsigned tag, const unsigned char* data, size_t len ) override;
    void handleResponseOfSubscribeScreen(const unsigned char* data, size_t len);
    void handlePublishedScreenFrame(const unsigned char* data, size_t len);
    void handlePublishCursorPosition(const unsigned char* data, size_t len);
    void handleCommandOfSetClipboardText(const unsigned char* data, size_t len);
    void handleResponseOfEcho(const unsigned char* data, size_t len);

    void whenConnecting(EasyIO::TCP::IConnection* con) override;
    void whenConnected(EasyIO::TCP::IConnection* con) override;
    void whenConnectFailed(EasyIO::TCP::IConnection* con, const std::string& reason) override;
    void whenDisconnected(EasyIO::TCP::IConnection* con) override;
    void whenLoggedIn() override;
    void whenLoginFailed(int errCode, const std::string& errString) override;

    void setPartnerClipboardText(const std::string& text);
    void setPartnerMousePosition(int x, int y);

    bool changePanelAutoScaleMode();
    bool changePanelOneToOneMode();

    void synchronizeMousePosition();

    void roseCurtain();
    void curtainDown();

    void printSpeed();
    void echo();
    void printTimeDelay();

    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();
    void handleError(int errCode, QString errString);

    void adjustDisplayerSize();
    void updateCurtainText(bool connected, bool loggedIn, bool authorized);

    void on_action1_1_triggered();

    void on_actionAutoScale_triggered();

    void on_actionPrintScreen_triggered();

    void on_actionCtrlShiftEsc_triggered();

public:
    std::function<void(ScreenViewer*)> onClose;

private:
    QTimer m_timerTick;

    unsigned m_wPartner = 0;
    unsigned m_hPartner = 0;

    int m_xSelfMousePos = 0;
    int m_ySelfMousePos = 0;

    bool m_selfMousePosChanged = false;
    long long m_lastMouseSynchronizingTime = 0;

    QLayout* m_panelLayout = nullptr;
    DesktopDisplayer *m_displayer = nullptr;
    int m_scaledPanel;

    SpeedCalculation m_speedCalculation;
    long long m_speed = 0;
    QLabel* m_lblSpeed = nullptr;
    long long m_lastPrintSpeedTime = 0;

    QLabel* m_lblTimeDelay = nullptr;
    long long m_timeDelay = -1;
    long long m_lastEchoTime = 0;
    long long m_lastPrintTimeDelayTime = -1;

    std::unique_ptr<Cryptology::AES<EasyIO::ByteBuffer>> m_aesDirect;
    std::unique_ptr<ISVCDecoder, ISVCDecoderDeleter> m_decoder;

    QString m_curtainText;
    bool m_curtainDowned = false;
    std::recursive_mutex m_mutex;

    Ui::ScreenViewer *ui;
};

#endif // SCREENVIEWER_H
