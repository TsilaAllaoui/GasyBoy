#ifndef _GAMEBOY_H_
#define _GAMEBOY_H_

#include <SDL2/SDL.h>
#include <vector>
#include "mmu.h"
#include "cpu.h"
#include "gpu.h"
#include "defs.h"
#include "timer.h"
#include "interrupt.h"
#include "gamepad.h"
#include "eventListener.h"

namespace gasyboy
{
    class Gpu;
    class Gamepad;
    class Interrupter;

    class GameBoy
    {
    private:
        std::shared_ptr<Mmu> mmu;
        std::shared_ptr<Cpu> cpu;
        std::shared_ptr<Gpu> gpu;
        std::shared_ptr<Timer> timer;
        std::shared_ptr<Gamepad> gamepad;
        std::shared_ptr<Interrupter> interruptHanlder;

        int cycleCounter;

        std::shared_ptr<SDL_Window> window;
        SDL_GLContext glcontext;
        SDL_Rect tile_map_pos, bg_map_pos;

    public:
        GameBoy(std::string filename);
        ~GameBoy();
        void boot();
        void step();
    };
}

#endif