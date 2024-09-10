#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "SDL.h"
#include "registers.h"
#include <chrono>

namespace gasyboy
{
    class Debugger
    {
    public:
        Debugger(Registers &registers);
        ~Debugger();
        void render();

    private:
        SDL_Window *_window;
        SDL_Renderer *_renderer;
        Registers &_registers;
    };
}

#endif