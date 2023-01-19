#include <QtGlobal>

#include "TerminalManager.h"
#include <quuid.h>
#ifdef Q_OS_WIN
    #include <Windows.h>
#else
    #include <pty.h>
#endif
#include "Global/Component/Logger/Logger.h"

TerminalManager::TerminalManager()
    :
#ifndef Q_OS_WIN
    m_hEventLoop(0),
#endif
    m_exit(0)
{

}


TerminalManager::~TerminalManager()
{
    exit();
    waitForExited();
}

TerminalManager *TerminalManager::create()
{
    TerminalManager *ret = new TerminalManager();
#ifndef Q_OS_WIN
    ret->m_hEventLoop = epoll_create(1);
    assert(ret->m_hEventLoop != -1);
#endif

     ret->m_thread = std::thread(std::bind(&TerminalManager::exec, ret));

     return ret;
}

void TerminalManager::exit()
{
    m_exit = true;
#ifndef Q_OS_WIN
    close(m_hEventLoop);
#endif
}

void TerminalManager::waitForExited()
{
    if (m_thread.joinable())
        m_thread.join();
}

std::string TerminalManager::createTerminal(User::ID userId, short width, short height, bool *isOk)
{
    bool b = false;
    std::string ret = QUuid::createUuid().toString(QUuid::Id128).toStdString();
    Item *item(new Item());
    item->id = ret;
    item->userId = userId;

#ifdef Q_OS_WIN
    do
    {
        bool fSuccess;
        CreatePipe(&item->inPipePseudoConsoleSide, &item->inPipeOurSide, NULL, 0);
        CreatePipe(&item->outPipeOurSide, &item->outPipePseudoConsoleSide, NULL, 0);

        if (CreatePseudoConsole(COORD{width, height}, item->inPipePseudoConsoleSide, item->outPipePseudoConsoleSide, 0, &item->hPC) != S_OK)
        {
            Logger::error("%s:%d - CreatePseudoConsole failed[%d]",  __PRETTY_FUNCTION__, __LINE__, GetLastError());
            break;
        }

        STARTUPINFOEXW  si;
        memset(&si, 0, sizeof(si));
        si.StartupInfo.cb = sizeof(STARTUPINFOEXW);


        size_t size;
        InitializeProcThreadAttributeList(NULL, 1, 0, &size);
        si.lpAttributeList = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(new BYTE[size]);
        fSuccess = InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, (PSIZE_T)&size);
        if (!fSuccess)
        {
            Logger::error("%s:%d - InitializeProcThreadAttributeList failed[%d]",  __PRETTY_FUNCTION__, __LINE__, GetLastError());
            DeleteProcThreadAttributeList(si.lpAttributeList);
            break;
        }

        fSuccess = UpdateProcThreadAttribute(
                                    si.lpAttributeList,
                                    0,
                                    PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                                    item->hPC,
                                    sizeof(item->hPC),
                                    NULL,
                                    NULL);
        if (!fSuccess)
        {
            Logger::error("%s:%d - UpdateProcThreadAttribute failed[%d]",  __PRETTY_FUNCTION__, __LINE__, GetLastError());
            DeleteProcThreadAttributeList(si.lpAttributeList);
            break;
        }

        std::wstring commandline = L"cmd.exe";
        fSuccess = CreateProcessW(
                        nullptr,
                        commandline.data(),
                        nullptr,
                        nullptr,
                        TRUE,
                        EXTENDED_STARTUPINFO_PRESENT,
                        nullptr,
                        nullptr,
                        &si.StartupInfo,
                        &item->pi);

        if (!fSuccess)
        {
            Logger::error("%s:%d - CreateProcessW failed[%d]",  __PRETTY_FUNCTION__, __LINE__, GetLastError());
            DeleteProcThreadAttributeList(si.lpAttributeList);
            break;
        }
        DeleteProcThreadAttributeList(si.lpAttributeList);

        Logger::info("new terminal %s, pid %ld", item->id.c_str(), item->pi.dwProcessId);
        m_mutex.lock();
        b = m_items.insert({item->id, item}).second;
        assert(b);
        m_mutex.unlock();

        b = true;

    } while (0);
#else
    do
    {
        if (m_hEventLoop == 0)
            break;

        __pid_t child = forkpty(&item->mpty, nullptr, nullptr, nullptr);
        if (child == -1)
        {
            Logger::error("%s:%d - forkpty failed[%d: %s]",  __PRETTY_FUNCTION__, __LINE__,
                          errno, strerror(errno));
            break;
        }
        item->pid = child;

        if (child == 0)
        {
            execl("/bin/login", "");
            return 0;
        }

        item->events = EPOLLIN;
        item->data.ptr = item;
        if(epoll_ctl(m_hEventLoop, EPOLL_CTL_ADD, item->mpty, item) == -1)
        {
            Logger::error("%s:%d - EPOLL_CTL_ADD failed[%d: %s]",  __PRETTY_FUNCTION__, __LINE__,
                          errno, strerror(errno));
            break;
        }

        Logger::info("new terminal %s, pid %ld", item->id.c_str(), item->pid);
        m_mutex.lock();
        assert(m_items.insert({item->id, item}).second);
        m_mutex.unlock();

        b = true;
    } while (0);
#endif

    if (!b)
        item->dec();

    if (isOk)
        *isOk = b;

    return ret;
}

void TerminalManager::writeCommand(const std::string &terminalId, const char *str, size_t len)
{
    m_mutex.lock();
    auto it = m_items.find(terminalId);
    if (it != m_items.end())
    {
        int L = len;
#ifdef Q_OS_WIN
        std::string str1(len * 2, 0);
        L = 0;
        for (int i = 0; i < len; i++)
        {
            char ch = str[i];
            if (ch == '\r')
                continue;
            if (ch == '\n')
            {
                str1.data()[L++] = '\r';
                str1.data()[L++] = '\n';
            }
            else
                str1.data()[L++] = ch;
        }
#endif
        Item* item = it->second;
        size_t offset = 0;
        do
        {
#ifdef Q_OS_WIN
            DWORD l;
            bool success = WriteFile(item->inPipeOurSide, str1.data() + offset, L, &l, nullptr);
            if (!success)
            {
                closeTerminal(item->id);
                break;
            }
#else
            int l = ::write(item->mpty, str + offset, len - offset);
            if (l ==  -1)
            {
                Logger::error("%s:%d - write failed, err[%d]: %s",  __PRETTY_FUNCTION__, __LINE__,
                              l, strerror(errno));
                closeTerminal(item->id);
                break;
            }
#endif

            offset += l;
            if (offset == L)
                break;
        } while (1);
    }
    m_mutex.unlock();
}

void TerminalManager::closeTerminal(const std::string &terminalId)
{
    Item* item = nullptr;
    m_mutex.lock();
    auto it = m_items.find(terminalId);
    if (it != m_items.end())
    {
        item = it->second;
        m_items.erase(it);
    }
    m_mutex.unlock();

    if (item)
    {
#ifndef Q_OS_WIN
        epoll_ctl(m_hEventLoop, EPOLL_CTL_DEL, item->mpty, item);
#endif

        if (onTerminalClosed)
            onTerminalClosed(item->userId, item->id);

        item->dec();
    }
}

void TerminalManager::resizeTerminal(const std::string &terminalId, short width, short height)
{
    m_mutex.lock();
    auto it = m_items.find(terminalId);
    if (it != m_items.end())
    {
#ifdef Q_OS_WIN
        ResizePseudoConsole(it->second->hPC, {width, height});
#else
    struct winsize ws = { (unsigned short)height, (unsigned short)width, 0, 0 };
    ioctl(it->second->mpty, TIOCSWINSZ, &ws);
#endif
    }
    m_mutex.unlock();
}

void TerminalManager::exec()
{
    const int BUF_LEN = 4096;
    std::unique_ptr<char> buf(new char[BUF_LEN]);

#ifdef Q_OS_WIN
    Item *item;

    while (!m_exit)
    {
        std::vector<Item*> badChannels;
        m_mutex.lock();
        for (auto it : m_items)
        {
            item = it.second;
            DWORD n = 0;
            bool fSuccess = PeekNamedPipe(item->outPipeOurSide, nullptr, 0, nullptr, &n, nullptr);
            if (!fSuccess)
            {
                Logger::info("!PeekNamedPipe: %x", GetLastError());
                badChannels.push_back(item);
                continue;
            }

            if (n)
            {
                if (n > BUF_LEN)
                    n = BUF_LEN;

                DWORD l;
                fSuccess = ReadFile(item->outPipeOurSide, buf.get(), n,  &l, nullptr);
                if (!fSuccess)
                {
                    Logger::info("!ReadFile: %x", GetLastError());
                    badChannels.push_back(item);
                    continue;
                }
                if (onTerminalOutputted)
                    onTerminalOutputted(item->userId, item->id, buf.get(), l);
            }
            else
            {
                DWORD err = WaitForSingleObject(item->pi.hThread, 0);
                if (err == WAIT_FAILED)
                {
                    badChannels.push_back(item);
                    continue;
                }

                if (err != WAIT_TIMEOUT)
                {
                    badChannels.push_back(item);
                    continue;
                }
            }
        }

        m_mutex.unlock();

        for (auto it : badChannels)
        {
            closeTerminal(it->id);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
#else
    int i, ret;
    epoll_event* events;

    const int MAX_EVENT = 256;
    events = new epoll_event[MAX_EVENT];
    while (!m_exit)
    {
        ret = epoll_wait(m_hEventLoop, events, MAX_EVENT, 100);
        if (ret == -1)
        {
            switch(errno)
            {
            case EINTR:
            case EAGAIN:
                continue;
            default:
                break;
            }
        }
        for (i = 0; i < ret; i++)
        {
            Item* it = (Item*)events[i].data.ptr;
            bool closed = true;
            do
            {
                if (it->events & EPOLLIN)
                {
                    int l = read(it->mpty, buf.get(), BUF_LEN);
                    if (l <= 0)
                        break;

                    if (onTerminalOutputted)
                        onTerminalOutputted(it->userId, it->id, buf.get(), l);
                }

                if (it->events & EPOLLERR)
                    break;

                closed = false;
            } while (0);

            if (closed)
            {
                closeTerminal(it->id);
            }
        }
    }
    delete[] events;
#endif

    m_mutex.lock();
    auto items = m_items;
    m_mutex.unlock();

    for (auto it : items)
    {
        closeTerminal(it.second->id);
    }
}
