#ifdef EMSCRIPTEN
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#else
#include "SDL.h"
#include "SDL_timer.h"
#endif

#include <stdlib.h>
#include <array>
#include <bitset>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <algorithm>
#include "interruptManager.h"
#include "cpu.h"
#include "mmu.h"
#include "ppu.h"

namespace gasyboy
{
    class Renderer
    {
    public:
        SDL_Window *_window;
        SDL_Renderer *_renderer;

        SDL_Texture *_viewportTexture;

        std::shared_ptr<Cpu> _cpu;
        std::shared_ptr<Ppu> _ppu;
        std::shared_ptr<Registers> _registers;
        std::shared_ptr<InterruptManager> _interruptManager;
        std::shared_ptr<Mmu> _mmu;

        // Viewport
        int _viewportWidth = 160;
        int _viewportHeight = 144;
        std::array<uint8_t, 160 * 144 * 4> _viewportPixels;
        SDL_Rect _viewportRect = {0, 0, _viewportWidth, _viewportHeight};

        int _windowHeight = _viewportHeight;
        int _windowWidth = _viewportWidth;

        int _framerateTime = 1000 / 60;
        std::chrono::steady_clock::time_point _startFrame;
        std::chrono::steady_clock::time_point _endFrame;
        void checkFramerate();

        void initWindow(int windowWidth, int windowHeight);

        int _frameCount = 0;
        std::chrono::steady_clock::time_point _fpsTimerStart = std::chrono::steady_clock::now();

    public:
        Renderer();
        ~Renderer();

        void render();

        virtual void init();

        virtual void draw();

        void reset();

        enum class ColorMode : uint8_t
        {
            NORMAL,
            RETRO,
            GREY
        };
    };
}