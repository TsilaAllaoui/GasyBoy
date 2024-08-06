#pragma once

#include <iostream>
#include "mmu.h"
#include "cpu.h"

class Cpu;
class Mmu;

class Interrupter
{
    private:
        bool MasterInterrupt;
        Mmu *mmu;
        Cpu *cpu;
    public:
        Interrupter(){};
        Interrupter(Mmu *pmmu, Cpu *pcpu);
        ~Interrupter();
        void handleInterrupts();
        void requestInterrupt(int id);
        void serviceInterrupt(int id);
};
