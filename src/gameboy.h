#ifndef _GAMEBOY_H_
#define _GAMEBOY_H_

#ifdef EMSCRIPTEN
#include <SDL2/SDL.h>
#else
#include "SDL.h"
#include "debugger.h"
#endif

#include "mmu.h"
#include "cpu.h"
#include "ppu.h"
#include "defs.h"
#include "timer.h"
#include "gamepad.h"
#include "renderer.h"
#include "interruptManager.h"
#include <mutex>
#include <condition_variable>

namespace gasyboy
{
    class GameBoy
    {
        Mmu &_mmu;
        Cpu &_cpu;
        Timer &_timer;
        Gamepad &_gamepad;
        InterruptManager &_interruptManager;
        Ppu &_ppu;
        std::unique_ptr<Renderer> _renderer;

        int _cycleCounter;

        SDL_Window *_window;
        SDL_Rect _tile_map_pos, _bg_map_pos;
#ifndef EMSCRIPTEN
        std::unique_ptr<Debugger> _debugger;
#endif
        bool _debugMode;

    public:
        GameBoy();
        GameBoy(const uint8_t *bytes, const size_t &romSize);
        ~GameBoy() = default;

        Registers _registers;

        // Start the emulator
        void boot();

        // Step the emulator
        void step();

        // Stop the emulator
        void stop();

#ifndef EMSCRIPTEN
        // Set debug mode
        void setDebugMode(const bool &debugMode);
#endif

        enum class State
        {
            RUNNING,
            STOPPED,
            PAUSED
        };

        static State state;

        // Used for the main loop
        void loop();
    };
}

#endif