#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "SDL.h"
#include "registers.h"
#include <chrono>
#include "timer.h"

namespace gasyboy
{
    class Debugger
    {
    public:
        Debugger(Mmu &_mmu, Registers &registers, Timer &timer, SDL_Window *mainWindow);
        ~Debugger();

        void render();
        void renderCpuDebugScreen();
        void renderTimerDebugScrenn();

        SDL_Window *_window;

        std::vector<uint16_t> _breakPoints;

    private:
        SDL_Renderer *_renderer;
        Registers &_registers;
        Mmu &_mmu;
        Timer _timer;

        std::map<char, char *> _bytesBuffers;
        std::map<std::string, char *> _wordsBuffers;
    };
}

#endif