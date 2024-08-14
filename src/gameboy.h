#ifndef _GAMEBOY_H_
#define _GAMEBOY_H_

#include "SDL.h"
#include "mmu.h"
#include "cpu.h"
#include "defs.h"
#include "timer.h"
#include "interruptManager.h"
#include "gamepad.h"

namespace gasyboy
{
    class GameBoy
    {
    private:
        Registers _registers;
        Mmu _mmu;
        Cpu _cpu;
        Timer _timer;
        Gamepad _gamepad;
        InterruptManager _interruptManager;

        int _cycleCounter;

    public:
        GameBoy(const std::string &filePath, const bool &bootBios);
        ~GameBoy();

        // Start the emulator
        void boot();

        // Step the emulator
        void step();
    };
}

#endif