#ifndef _TIMER_H_
#define _TIMER_H_

#include <cstdint>

namespace gasyboy
{
    class InterruptManager;

    class Timer
    {
        InterruptManager &_interruptManager;

    public:
        Timer(InterruptManager &interruptManager);
        ~Timer() = default;

        // Timer registers
        static uint8_t _div;
        static uint8_t _tima;
        static uint8_t _tma;
        static uint8_t _tac;

        static int _divIncrementRate;
        static uint16_t _timaIncrementRate;

        static uint8_t DIV();
        static uint8_t TIMA();
        static uint8_t TMA();
        static uint8_t TAC();

        static void setDIV(const uint8_t &value);
        static void setTIMA(const uint8_t &value);
        static void setTMA(const uint8_t &value);
        static void setTAC(const uint8_t &value);

        void update(const int &cycles);

        static void updateDIV(const int &cycle);
        static void resetDIV();
        static void updateTIMA(const int &cycle);
        static void updateTMA(const int &value);
        static void updateTAC(const uint8_t &value);

        void stopTimer();
    };
}

#endif