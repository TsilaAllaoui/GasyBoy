#ifndef _GAMEBOY_H_
#define _GAMEBOY_H_

#ifdef __EMSCRIPTEN__
#include <SDL2/SDL.h>
#else
#include "SDL.h"
#endif

#include "mmu.h"
#include "cpu.h"
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
        Mmu _mmu;
        Cpu _cpu;
        Timer _timer;
        Gamepad _gamepad;
        InterruptManager _interruptManager;
        Ppu _ppu;
        Renderer *_renderer;

        int _cycleCounter;

        SDL_Window *_window;
        SDL_Rect _tile_map_pos, _bg_map_pos;

        bool _debugMode;

    public:
        GameBoy(const std::string &filePath, const bool &bootBios, const bool &debugMode = false);
        GameBoy(const uint8_t *bytes, const size_t &romSize, const bool &bootBios, const bool &debugMode = false);
        ~GameBoy();

        Registers _registers;

        // Start the emulator
        void boot();

        // Step the emulator
        void step();

        enum class State
        {
            RUNNING,
            STOPPED,
            PAUSED
        };

        // State of the emulator
        static State state;

        // Used for the main loop
        void loop();
    };
}

#endif