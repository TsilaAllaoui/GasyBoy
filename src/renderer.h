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

    public:
        Renderer();

        void render();

        virtual void init();

        virtual void draw();

        enum class ColorMode : uint8_t
        {
            NORMAL,
            RETRO,
            GREY
        };
    };

    class DebugRenderer : public Renderer
    {
    private:
        SDL_Texture *debugTexture;
        SDL_Texture *backgroundTexture;
        SDL_Texture *tilemapTexture;
        SDL_Texture *spritemapTexture;

        // Tilemap
        int tilemapWidth = 128;
        int tilemapHeight = 256;
        std::array<uint8_t, 128 * 256 * 4> tilemap_pixels;
        SDL_Rect tilemapRect = {0, _viewportHeight, tilemapWidth, tilemapHeight};

        // Spritemap
        int spritemapHeight = 64;
        int spritemapWidth = 40;
        std::array<uint8_t, 64 * 40 * 4> spritemap_pixels;
        SDL_Rect spritemapRect = {tilemapWidth, _viewportHeight,
                                  spritemapWidth * 2, spritemapHeight * 2};

        // VRAM
        int backgroundWidth = 256;
        int backgroundHeight = 256;
        std::array<uint8_t, 256 * 256 * 4> background_pixels;
        SDL_Rect background_rect = {tilemapWidth + spritemapWidth * 2,
                                    _viewportHeight, backgroundWidth,
                                    backgroundHeight};

        int windowHeight = _viewportHeight + backgroundHeight;
        int windowWidth = backgroundWidth + tilemapWidth + spritemapWidth * 2;

        void draw() override;
        void drawBackground();
        void drawTilemap();
        void drawSpritemap();
        void drawBackgroundOverflow();

        // Helper functions
        void drawRectangle(int x, int y, int width, int height, Colour color);

    public:
        using Renderer::Renderer;

        void init() override;
    };
}