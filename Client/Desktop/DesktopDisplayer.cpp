#include "DesktopDisplayer.h"
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>
#include <GL/gl.h>
#include <GL/glu.h>     //libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev
#include <QOpenGLBuffer>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPainter>
#include "Global/Component/Logger/Logger.h"

#define Z_NEAR  1

DesktopDisplayer::DesktopDisplayer(QWidget *parent)
    :   QOpenGLWidget(parent),
        m_z(-Z_NEAR),
        m_displayMode(SCALED)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
    this->installEventFilter(this);
    QObject::connect(this, SIGNAL(frameFilled()), this, SLOT(update()));
}

DesktopDisplayer::~DesktopDisplayer()
{

}

EasyIO::ByteBuffer DesktopDisplayer::getScreenBuffer()
{
    return m_screenBuffer;
}

void DesktopDisplayer::setScreenBuffer(EasyIO::ByteBuffer buf, unsigned textureWidth, unsigned textureHeight,
                                       unsigned screenWidth, unsigned screenHeihgt)
{
    size_t L = textureWidth * textureHeight * 4;
    assert(buf.numReadableBytes() >= L);

    std::lock_guard g(m_mutex);
    m_screenBuffer = buf;
    m_textureWidth = textureWidth;
    m_textureHeight = textureHeight;
    m_imageWidth = screenWidth;
    m_imageHeihgt = screenHeihgt;
    updateGeometry();
}

void DesktopDisplayer::beginFillFrame()
{
    m_mutex.lock();
}

void DesktopDisplayer::endFillFrame()
{
    m_mutex.unlock();
    emit frameFilled();
}

unsigned DesktopDisplayer::textureWidth()
{
    return m_textureWidth;
}

unsigned DesktopDisplayer::textureHeight()
{
    return m_textureHeight;
}

void DesktopDisplayer::changeDisplayMode(DisplayMode mode)
{
    std::lock_guard g(m_mutex);
    m_displayMode = mode;
    updateGeometry();
}

void DesktopDisplayer::initializeGL()
{
    OpenGLFunctions.initializeOpenGLFunctions();
    glClearColor(0.5, 0.5, 0.5, 0.5);
    glClearDepth(1.0f);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // screen
    OpenGLFunctions.glGenTextures(1, &m_textures[0]);
    glBindTexture(GL_TEXTURE_2D, m_textures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

}

void DesktopDisplayer::resizeGL(int w, int h)
{
    if (h == 0)
        h = 1;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_PROJECTION);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    updateGeometry();
}

void DesktopDisplayer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0, 0, -1);
    m_mutex.lock();
    if (m_screenBuffer.numReadableBytes())
    {
        glBindTexture(GL_TEXTURE_2D, m_textures[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_screenBuffer.readableBytes());
        glBegin(GL_QUADS);
                glTexCoord2f(0.0f, m_hProportion);                  glVertex2f(m_left, m_buttom);
                glTexCoord2f(m_wProportion, m_hProportion);         glVertex2f(m_right, m_buttom);
                glTexCoord2f(m_wProportion, 0.0f);                           glVertex2f(m_right, m_top);
                glTexCoord2f(0.0f, 0.0f);                  glVertex2f(m_left, m_top);
        glEnd();
    }
    m_mutex.unlock();
}

void DesktopDisplayer::wheelEvent(QWheelEvent *event)
{
    if (onWheel)
        onWheel(event->angleDelta().ry());
}

void DesktopDisplayer::mouseMoveEvent(QMouseEvent *event)
{
    int x, y;
    auto p = event->pos();
    toPartnerWindowsPosition(p.x(), p.y(), x, y);

    if (onMouseMove)
        onMouseMove(x, y);
}

void DesktopDisplayer::enterEvent(QEnterEvent *)
{
#ifdef Q_OS_WIN
    if (0)
        ShowCursor(false);
#endif
}

void DesktopDisplayer::leaveEvent(QEvent *event)
{
#ifdef Q_OS_WIN
    if (0)
        ShowCursor(true);
#endif
}

void DesktopDisplayer::mouseDoubleClickEvent(QMouseEvent *event)
{
    int x, y;
    auto p = event->pos();
    toPartnerWindowsPosition(p.x(), p.y(), x, y);

    if (onMouseDoubleClick)
        onMouseDoubleClick(event->button(), x, y);
}

void DesktopDisplayer::mousePressEvent(QMouseEvent *event)
{
    int x, y;
    auto p = event->pos();
    toPartnerWindowsPosition(p.x(), p.y(), x, y);

    if (onMousePress)
        onMousePress(event->button(), x, y);
}

void DesktopDisplayer::mouseReleaseEvent(QMouseEvent *event)
{
    int x, y;
    auto p = event->pos();
    toPartnerWindowsPosition(p.x(), p.y(), x, y);

    if (onMouseRelease)
        onMouseRelease(event->button(), x, y);
}

void DesktopDisplayer::keyPressEvent(QKeyEvent *event)
{
    if (onKeyPress)
        onKeyPress((Qt::Key)event->key());
}

void DesktopDisplayer::keyReleaseEvent(QKeyEvent *event)
{
    if (onKeyRelease)
        onKeyRelease((Qt::Key)event->key());
}

bool DesktopDisplayer::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *event1 = dynamic_cast<QKeyEvent*>(event);
        assert(event1);
        if (event1->key() == Qt::Key::Key_Tab)
        {
            if (event->type() == QEvent::KeyPress)
            {
                keyPressEvent(event1);
            }
            else
            {
                keyReleaseEvent(event1);
            }
            return true;
        }
    }

    return QOpenGLWidget::eventFilter(object, event);
}

void DesktopDisplayer::updateGeometry()
{
    int W = width();
    int H = height();
    float w = 0, h = 0, d = 0;

    m_wProportion = (double)m_imageWidth / m_textureWidth;
    m_hProportion = (double)m_imageHeihgt / m_textureHeight;

    if (m_displayMode == ONE_TO_ONE)
    {
        m_left = -1;
        m_top = 1;
        m_right = 1;
        m_buttom = -1;
    }
    else if (m_displayMode == SCALED)
    {
        h = m_imageHeihgt / ((double)m_imageWidth / W);
        if (h < H)
        {
            m_left = -1;
            m_right = 1;
            float d = h / H;
            m_top = d;
            m_buttom = -d;
        }
        else
        {
            m_top = 1;
            m_buttom = -1;

            w = m_imageWidth / ((double)m_imageHeihgt / H);
            d = w / W;
            m_left = -d;
            m_right = d;
        }
    }
    else
        assert(false);
}

void DesktopDisplayer::toPartnerWindowsPosition(int inX, int inY, int &outX, int &outY)
{
    outX = outY = 0;

    int W = width();
    int H = height();

    if (!W || !H)
        return;

    float l = (W / 2) * (1 + m_left);
    float r = (W / 2) * (1 + m_right);
    outX = (inX - l) / (r - l) * m_imageWidth;

    float t = (H / 2) * (1 - m_top);
    float b = (H / 2) * (1 - m_buttom);
    outY = (inY - t) / (b - t) * m_imageHeihgt;
}
