#pragma once

#include <SDL2/SDL.h>
#include <vector>
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
        //Gpu *gpu;
        Timer *timer;
        Gamepad *gamepad;
        Interrupter *interruptHanlder;

        int cycleCounter;

		SDL_Window *screen, *window, *tileMap;
		SDL_GLContext glcontext;
        SDL_Rect tile_map_pos, bg_map_pos;
		SDL_Renderer *screenRenderer, *tileMapRenderer;

    public:
        GameBoy(string filename);
        ~GameBoy();
        void boot();
        void step();
};