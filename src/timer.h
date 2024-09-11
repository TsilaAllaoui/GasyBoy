#ifndef _TIMER_H_
#define _TIMER_H_

#include "interruptManager.h"
#include "mmu.h"

namespace gasyboy
{
    class Timer
    {
        Mmu &_mmu;
        InterruptManager &_interruptManager;

        // Timer registers
        uint8_t _div;
        uint8_t _tima;
        uint8_t _tma;
        uint8_t _tac;

        uint16_t _divIncrementRate;
        uint16_t _timaIncrementRate;

    public:
        Timer(Mmu &mmu, InterruptManager &interruptManager);
        ~Timer() = default;

        uint8_t DIV();
        uint8_t TIMA();
        uint8_t TMA();
        uint8_t TAC();

        void update(const int &cycles);

        void updateDIV(const int &cycle);
        void resetDIV();
        void updateTIMA(const int &cycle);
        void updateTMA(const int &value);
        void updateTAC(const uint8_t &value);

        void stopTimer();
    };
}

#endif