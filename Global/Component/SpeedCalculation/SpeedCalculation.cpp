#include "SpeedCalculation.h"

#define NUM_SLOT_PRE_SECOND 10
#define MECS_PER_SLOT   (1000 / NUM_SLOT_PRE_SECOND)

SpeedCalculation::SpeedCalculation()
    :   m_items(NUM_SLOT_PRE_SECOND + 2)
{
    for (int i = 0; i < m_items.size()  - 1; i++)
    {
        m_items[i].next = &m_items[i + 1];
    }
    m_items.back().next = &m_items.front();

    for (int i = m_items.size()  - 1; i > 0; i--)
    {
        m_items[i].previous = &m_items[i - 1];
    }
    m_items.front().previous = &m_items.back();
    m_current = &m_items.front();
}

void SpeedCalculation::update(long long msec, size_t value)
{
    long long t = convertTime(msec);
    if (m_current->time != t)
    {
        m_current = m_current->next;
        m_current->time = t;
        m_current->total = 0;
    }

    m_current->total += value;
}

long long SpeedCalculation::get()
{
    long long l = m_current->time - convertTime(1000);
    Node* p = m_current, *end = m_current;
    long long ret = 0;
    while (p->time > l)
    {
        ret += p->total;
        p = p->previous;
        if (p == end)
            break;
    }
    return ret;
}

long long SpeedCalculation::convertTime(long long t)
{
    t = t / MECS_PER_SLOT;
    return t;
}


