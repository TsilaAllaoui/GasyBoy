#ifndef _GAMEBOY_H_
#define _GAMEBOY_H_

#include "SDL.h"
#include "mmu.h"
#include "cpu.h"
// #include "gpu.h"
#include "ppu.h"
#include "defs.h"
#include "timer.h"
#include "gamepad.h"
#include "renderer.h"
#include "interruptManager.h"

namespace gasyboy
{
    class GameBoy
    {
    private:
        Registers _registers;
        Mmu _mmu;
        Cpu _cpu;
        // Gpu _gpu;
        Timer _timer;
        Gamepad _gamepad;
        InterruptManager _interruptManager;
        Ppu _ppu;
        Renderer _renderer;

        int _cycleCounter;

        SDL_Window *_window;
        SDL_Rect _tile_map_pos, _bg_map_pos;

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