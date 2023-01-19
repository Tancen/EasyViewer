#include "Clipboard.h"
#include <QApplication>
#include <qclipboard.h>
#include <QMimeData>

Clipboard Clipboard::s_this;

Clipboard *Clipboard::share()
{
    return &s_this;
}

bool Clipboard::init()
{
    QObject::connect(QApplication::clipboard(), SIGNAL(dataChanged()),
                            &s_this, SLOT(whenClipboardDataChanged()));
    QObject::connect(&s_this, SIGNAL(textChanged(QString)),
                            &s_this, SLOT(_setText(QString)));

    return true;
}

void Clipboard::setText(const std::string &text)
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    std::string s;
    try
    {
        s = QApplication::clipboard()->text().toStdString();
    }  catch (...)
    {
        return;
    }
    if (text != s)
    {
        m_text = text;
        emit textChanged(text.c_str());
    }
}

void Clipboard::subscribe(void *key, Clipboard::HitCallback hitCallback)
{
    assert(hitCallback);
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    m_followers.insert({key, hitCallback});
}

void Clipboard::unsubscribe(void *key)
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    m_followers.erase(key);
}

Clipboard::Clipboard()
{

}

Clipboard::~Clipboard()
{

}

void Clipboard::whenClipboardDataChanged()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    if (mimeData != nullptr && mimeData->hasText())
    {
        std::string text = mimeData->text().toStdString();
        if (text.empty())
            return;

        std::lock_guard<std::recursive_mutex> guard(m_mutex);
        if (text != m_text)
        {
            m_text = text;
            for (auto it : m_followers)
                it.second(m_text);
        }
    }
}

void Clipboard::_setText(QString text)
{
    if (text.isEmpty())
        return;
    QApplication::clipboard()->setText(text);
}
