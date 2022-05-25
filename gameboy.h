#pragma once

#include <SDL/SDL.h>
#include "mmu.h"
#include "cpu.h"
#include "gpu.h"
#include "defs.h"
#include "timer.h"
#include "interrupt.h"
#include "gamepad.h"

class Gpu;
class Gamepad;
class Interrupter;

class GameBoy
{
    private:
        Mmu *mmu;
        Cpu *cpu;
        Gpu *gpu;
        int cycle_counter;
        SDL_Surface *screen, *window, *tile_map, *bg_map;
        SDL_Rect tile_map_pos, bg_map_pos, window_pos;
        Timer *timer;
        Gamepad *gamepad;
        Interrupter *interruptHanlder;

    public:
        GameBoy(string filename);
        ~GameBoy();
        void boot();
        void step();
};