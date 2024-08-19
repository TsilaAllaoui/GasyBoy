#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include "SDL.h"

#define DEBUGGER_SCREEN_WIDTH 240
#define DEBUGGER_SCREEN_HEIGHT 240

namespace gasyboy
{
    class Debugger
    {
        SDL_Window *_imguiWindow;
        SDL_Renderer *_imguiRenderer;

    public:
        Debugger();
        ~Debugger();

        void render();
    };
}

#endif