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

        int _divideCounter;

        int _timerCounter;

    public:
        Timer(Mmu &mmu, InterruptManager &interruptManager);
        ~Timer() = default;

        bool isTimerOn();

        void updateFrequence();

        void updateDivider(const int &cycle);

        void updateTimer(const int &cycle);
    };
}

#endif