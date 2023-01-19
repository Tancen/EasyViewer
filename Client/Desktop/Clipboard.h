#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <functional>
#include <thread>
#include <mutex>
#include <map>
#include <QObject>

class Clipboard : public QObject
{
    Q_OBJECT

public:
    typedef std::function<void(const std::string& text)> HitCallback;
public:
    static Clipboard* share();
    static bool init();

    void setText(const std::string& text);
    void subscribe(void* key, HitCallback hitCallback);
    void unsubscribe(void* key);


signals:
    void textChanged(QString text);

private:
    Clipboard();
    ~Clipboard();

private slots:
    void whenClipboardDataChanged();
    void _setText(QString text);

private:
    std::recursive_mutex m_mutex;
    std::map<void*, HitCallback> m_followers;

    std::string m_text;

    static Clipboard s_this;
};


#endif // CLIPBOARD_H
