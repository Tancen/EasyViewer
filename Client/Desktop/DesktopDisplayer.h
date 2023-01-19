#ifndef DESKTOPDISPLAYER_H
#define DESKTOPDISPLAYER_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <mutex>
#include <QImage>
#include "Global/Component/EasyIO/EasyIOByteBuffer.h"

class DesktopDisplayer : public QOpenGLWidget
{
    Q_OBJECT

signals:
    void frameFilled();

public:
    enum DisplayMode { ONE_TO_ONE, SCALED };

public:
    DesktopDisplayer(QWidget* parent = nullptr);
    ~DesktopDisplayer();

    EasyIO::ByteBuffer getScreenBuffer();
    void setScreenBuffer(EasyIO::ByteBuffer buf, unsigned textureWidth, unsigned textureHeight,
                         unsigned screenWidth, unsigned screenHeihgt);
    void beginFillFrame();
    void endFillFrame();

    unsigned textureWidth();
    unsigned textureHeight();

    void changeDisplayMode(DisplayMode mode);

private:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void wheelEvent(QWheelEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    bool eventFilter(QObject *object, QEvent *event) override;

    void updateGeometry();
    void toPartnerWindowsPosition(int inX, int inY, int& outX, int& outY);

public:
    std::function<void(int angleDelta)> onWheel;
    std::function<void(int x, int y)> onMouseMove;
    std::function<void(Qt::MouseButton button, int x, int y)> onMouseDoubleClick;
    std::function<void(Qt::MouseButton button, int x, int y)> onMousePress;
    std::function<void(Qt::MouseButton button, int x, int y)> onMouseRelease;
    std::function<void(Qt::Key key)> onKeyPress;
    std::function<void(Qt::Key key)> onKeyRelease;

private:
    EasyIO::ByteBuffer m_screenBuffer;

    double m_z;
    unsigned m_textures[1];

    QOpenGLFunctions OpenGLFunctions;

    unsigned m_textureWidth = 1;
    unsigned m_textureHeight = 1;
    unsigned m_imageWidth = 1;
    unsigned m_imageHeihgt = 1;

    float m_wProportion;
    float m_hProportion;
    float m_left;
    float m_top;
    float m_right;
    float m_buttom;
    DisplayMode m_displayMode;
    std::recursive_mutex m_mutex;
};


#endif // DESKTOPDISPLAYER_H
