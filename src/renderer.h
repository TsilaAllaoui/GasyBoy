#include "SDL.h"
#include "SDL_timer.h"
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

        Cpu &_cpu;
        Ppu &_ppu;
        Registers &_registers;
        InterruptManager &_interruptManager;
        Mmu &_mmu;

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
        void drawViewport();

    public:
        Renderer(Cpu &cpu,
                 Ppu &ppu,
                 Registers &registers,
                 InterruptManager &interruptManager,
                 Mmu &mmu);

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
}

// class DebugRenderer : public Renderer
// {
// private:
//     SDL_Texture *debug_texture;
//     SDL_Texture *background_texture;
//     SDL_Texture *tilemap_texture;
//     SDL_Texture *spritemap_texture;

//     // Tilemap
//     int tilemap_width = 128;
//     int tilemap_height = 256;
//     std::array<uint8_t, 128 * 256 * 4> tilemap_pixels;
//     SDL_Rect tilemap_rect = {0, viewport_height, tilemap_width, tilemap_height};

//     // Spritemap
//     int spritemap_height = 64;
//     int spritemap_width = 40;
//     std::array<uint8_t, 64 * 40 * 4> spritemap_pixels;
//     SDL_Rect spritemap_rect = {tilemap_width, viewport_height,
//                                spritemap_width * 2, spritemap_height * 2};

//     // VRAM
//     int background_width = 256;
//     int background_height = 256;
//     std::array<uint8_t, 256 * 256 * 4> background_pixels;
//     SDL_Rect background_rect = {tilemap_width + spritemap_width * 2,
//                                 viewport_height, background_width,
//                                 background_height};

//     int window_height = viewport_height + background_height;
//     int window_width = background_width + tilemap_width + spritemap_width * 2;

//     void draw() override;
//     void draw_background();
//     void draw_tilemap();
//     void draw_spritemap();
//     void draw_status();
//     void draw_background_overflow();

//     // Helper functions
//     void draw_text(int x_pop, int y_pos, std::string text);
//     void draw_rectangle(int x, int y, int width, int height, Colour color);

// public:
//     using Renderer::Renderer;

//     void init() override;
// };
