#ifndef SPEEDCALCULATION_H
#define SPEEDCALCULATION_H

#include <vector>

class SpeedCalculation
{
    struct Node
    {
        size_t total = 0;
        long long time = 0;
        Node* next = nullptr;
        Node* previous = nullptr;
    };
public:
    SpeedCalculation();

    /**
    * @brief   Increase a value for the specified time
    * @param   msec the specified time in milliseconds
    * @param   value the amount for increasing
    */
    void update(long long msec, size_t value);

    /**
    * @brief   Get the total value of the last update time in seconds
    */
    long long get();

private:
    inline long long convertTime(long long t);

private:
    std::vector<Node> m_items;
    Node* m_current = nullptr;
};

#endif // SPEEDCALCULATION_H
