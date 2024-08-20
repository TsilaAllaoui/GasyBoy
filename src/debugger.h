#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include "SDL.h"
#include "registers.h"

#define DEBUGGER_SCREEN_WIDTH 480
#define DEBUGGER_SCREEN_HEIGHT 480

#define REGISTERS_WINDOW_WIDTH 175
#define REGISTERS_WINDOW_HEIGHT 180

namespace gasyboy
{
    class Debugger
    {
        SDL_Window *_imguiWindow;
        SDL_Renderer *_imguiRenderer;

        Registers &_registers;

    public:
        Debugger(Registers &registers);
        ~Debugger();

        void render();
    };
}

#endif