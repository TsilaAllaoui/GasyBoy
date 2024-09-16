#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "SDL.h"
#include "registers.h"
#include <chrono>
#include "timer.h"
#include <functional>

namespace gasyboy
{
    class Debugger
    {
    public:
        Debugger(Mmu &_mmu, Registers &registers, Timer &timer, SDL_Window *mainWindow);
        ~Debugger();

        void render();
        void renderCpuDebugScreen();
        void renderTimerDebugScreen();
        void renderJoypadDebugScreen();

        SDL_Window *_window;

        std::vector<uint16_t> _breakPoints;

    private:
        SDL_Renderer *_renderer;
        Registers &_registers;
        Mmu &_mmu;
        Timer _timer;

        std::map<std::string, char *> _bytesBuffers;
        std::map<std::string, char *> _wordsBuffers;
        void renderByte(const std::string &reg, std::function<uint8_t()> get, std::function<void(uint8_t)> set);
        void renderWord(const std::string &reg, std::function<uint16_t()> get, std::function<void(uint16_t)> set);

        std::vector<std::pair<std::string, bool>> _buttons;
        std::vector<std::pair<std::string, bool>> _directions;
    };
}

#endif