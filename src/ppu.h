#ifndef PPU_H
#define PPU_H

#include "SDL.h"
#include "mmu.h"

namespace gasyboy
{
    // Define LCD modes
#define MODE_HBLANK 0
#define MODE_VBLANK 1
#define MODE_OAM 2
#define MODE_TRANSFER 3

// Timing constants
#define SCANLINE_CYCLES 456
#define OAM_CYCLES 80
#define TRANSFER_CYCLES 172
#define HBLANK_CYCLES (SCANLINE_CYCLES - OAM_CYCLES - TRANSFER_CYCLES)
#define VBLANK_SCANLINES 10

    class Ppu
    {
    public:
        Ppu(Mmu &mmu);
        ~Ppu();

        void step(const int &cycle); // Step through Ppu cycles
        void render();               // Render the frame

    private:
        Mmu &mmu;
        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_Texture *texture;
        uint32_t pixels[160 * 144]; // GB resolution: 160x144

        void initializeSDL();
        void destroySDL();
        void drawPixel(int x, int y, uint32_t color);
        void renderScanline(int scanline);
        void renderBackground(int scanline); // Renders the background layer for the specified scanline
        void renderWindow(int scanline);     // Renders the window layer for the specified scanline
        void renderSprites(int scanline);    // Renders the sprites for the specified scanline
        uint32_t getColorFromPalette(int colorBit);
    };
}

#endif // PPU_H