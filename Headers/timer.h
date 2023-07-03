#pragma once

#include <iostream>
#include "mmu.h"

class Timer
{
    private:
        Mmu *mmu;
        int divide_counter, timer_counter;
    public:
        Timer();
        Timer(Mmu *p_mmu);
        ~Timer();
        int timer_on();
        void setFreq();
        void divider(int m_cycle);
        void updateTimer(int m_cycle);
        void change_interrupt(bool value);
        bool get_interrupt_status();
        void requestInterrupt(int id);
};
