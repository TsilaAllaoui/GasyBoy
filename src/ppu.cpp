#include "ppu.h"
#include <stdexcept>

namespace gasyboy
{
    Ppu::Ppu(Mmu &mmu) : mmu(mmu), window(nullptr), renderer(nullptr), texture(nullptr)
    {
        initializeSDL();
    }

    Ppu::~Ppu()
    {
        destroySDL();
    }

    void Ppu::initializeSDL()
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
        }

        window = SDL_CreateWindow("Game Boy Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 160 * SCALE, 144 * SCALE, SDL_WINDOW_SHOWN);
        if (!window)
        {
            throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer)
        {
            throw std::runtime_error("Failed to create SDL renderer: " + std::string(SDL_GetError()));
        }

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
        if (!texture)
        {
            throw std::runtime_error("Failed to create SDL texture: " + std::string(SDL_GetError()));
        }
    }

    void Ppu::destroySDL()
    {
        if (texture)
        {
            SDL_DestroyTexture(texture);
        }
        if (renderer)
        {
            SDL_DestroyRenderer(renderer);
        }
        if (window)
        {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
    }

    void Ppu::step(const int &cycle)
    {
        static int cycleCount = 0;
        static int currentMode = MODE_OAM;
        static int currentScanline = 0;

        cycleCount += cycle;

        switch (currentMode)
        {
        case MODE_OAM:
            if (cycleCount >= OAM_CYCLES)
            {
                cycleCount -= OAM_CYCLES;
                currentMode = MODE_TRANSFER;
            }
            break;
        case MODE_TRANSFER:
            if (cycleCount >= TRANSFER_CYCLES)
            {
                cycleCount -= TRANSFER_CYCLES;
                currentMode = MODE_HBLANK;

                // Render the current scanline
                renderBackground(currentScanline);
                renderWindow(currentScanline);
                renderSprites(currentScanline);
            }
            break;
        case MODE_HBLANK:
            if (cycleCount >= HBLANK_CYCLES)
            {
                cycleCount -= HBLANK_CYCLES;
                currentScanline++;
                if (currentScanline >= 144)
                {
                    currentMode = MODE_VBLANK;
                    render();
                }
                else
                {
                    currentMode = MODE_OAM;
                }
            }
            break;
        case MODE_VBLANK:
            if (cycleCount >= SCANLINE_CYCLES)
            {
                cycleCount -= SCANLINE_CYCLES;
                currentScanline++;
                if (currentScanline >= 144 + VBLANK_SCANLINES)
                {
                    currentScanline = 0;
                    currentMode = MODE_OAM;
                }
            }
            break;
        }
    }

    void Ppu::renderScanline(int scanline)
    {
        // Simplified: fill the scanline with a pattern or color
        for (int x = 0; x < 160; ++x)
        {
            uint32_t color = (scanline % 2 == 0) ? 0xFFFFFFFF : 0xFFAAAAAA; // Alternate colors for demonstration
            drawPixel(x, scanline, color);
        }
    }

    void Ppu::render()
    {
        SDL_Rect dstRect = {0, 0, 160 * SCALE, 144 * SCALE};
        SDL_UpdateTexture(texture, nullptr, pixels, 160 * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
        SDL_RenderPresent(renderer);
    }

    void Ppu::drawPixel(int x, int y, uint32_t color)
    {
        if (x >= 0 && x < 160 && y >= 0 && y < 144)
        {
            pixels[y * 160 + x] = color;
        }
    }

    void Ppu::renderBackground(int scanline)
    {
        // Get scroll values
        uint8_t scrollY = mmu.readRam(0xFF42);
        uint8_t scrollX = mmu.readRam(0xFF43);

        // Determine which tile row to render
        int tileRow = ((scanline + scrollY) / 8) % 32;

        for (int pixelX = 0; pixelX < 160; ++pixelX)
        {
            int tileCol = ((pixelX + scrollX) / 8) % 32;

            // Get tile ID from the background map
            uint16_t bgMapAddr = mmu.readRam(0xFF40) & (1 << 6) ? 0x9C00 : 0x9800;
            uint8_t tileID = mmu.readRam(bgMapAddr + tileRow * 32 + tileCol);

            // Get tile data (assuming unsigned addressing)
            uint16_t tileDataAddr = 0x8000 + tileID * 16;
            int tileY = (scanline + scrollY) % 8;
            uint8_t tileData1 = mmu.readRam(tileDataAddr + tileY * 2);
            uint8_t tileData2 = mmu.readRam(tileDataAddr + tileY * 2 + 1);

            int tileX = (pixelX + scrollX) % 8;
            int colorBit = (tileData1 & (0x80 >> tileX)) ? 1 : 0;
            colorBit |= (tileData2 & (0x80 >> tileX)) ? 2 : 0;

            uint32_t color = getColorFromPalette(colorBit);

            drawPixel(pixelX, scanline, color);
        }
    }

    void Ppu::renderWindow(int scanline)
    {
        uint8_t windowY = mmu.readRam(0xFF4A);
        uint8_t windowX = mmu.readRam(0xFF4B) - 7;

        if (scanline >= windowY)
        {
            int windowLine = scanline - windowY;

            for (int pixelX = windowX; pixelX < 160; ++pixelX)
            {
                int tileCol = (pixelX - windowX) / 8;
                int tileRow = windowLine / 8;

                // Get tile ID from the window map
                uint16_t winMapAddr = 0x9800; // Can be 0x9800 or 0x9C00
                uint8_t tileID = mmu.readRam(winMapAddr + tileRow * 32 + tileCol);

                // Get tile data (assuming unsigned addressing)
                uint16_t tileDataAddr = 0x8000 + tileID * 16;
                int tileY = windowLine % 8;
                uint8_t tileData1 = mmu.readRam(tileDataAddr + tileY * 2);
                uint8_t tileData2 = mmu.readRam(tileDataAddr + tileY * 2 + 1);

                int tileX = (pixelX - windowX) % 8;
                int colorBit = (tileData1 & (0x80 >> tileX)) ? 1 : 0;
                colorBit |= (tileData2 & (0x80 >> tileX)) ? 2 : 0;

                uint32_t color = getColorFromPalette(colorBit);

                drawPixel(pixelX, scanline, color);
            }
        }
    }

    void Ppu::renderSprites(int scanline)
    {
        for (int i = 0; i < 40; ++i)
        { // Max 40 sprites
            uint8_t spriteY = mmu.readRam(0xFE00 + i * 4) - 16;
            uint8_t spriteX = mmu.readRam(0xFE00 + i * 4 + 1) - 8;
            uint8_t tileID = mmu.readRam(0xFE00 + i * 4 + 2);
            uint8_t attributes = mmu.readRam(0xFE00 + i * 4 + 3);

            if (scanline >= spriteY && scanline < spriteY + 8)
            {
                int tileY = scanline - spriteY;
                uint16_t tileDataAddr = 0x8000 + tileID * 16;
                uint8_t tileData1 = mmu.readRam(tileDataAddr + tileY * 2);
                uint8_t tileData2 = mmu.readRam(tileDataAddr + tileY * 2 + 1);

                for (int tileX = 0; tileX < 8; ++tileX)
                {
                    int colorBit = (tileData1 & (0x80 >> tileX)) ? 1 : 0;
                    colorBit |= (tileData2 & (0x80 >> tileX)) ? 2 : 0;

                    uint32_t color = getColorFromPalette(colorBit);

                    drawPixel(spriteX + tileX, scanline, color);
                }
            }
        }
    }

    uint32_t Ppu::getColorFromPalette(int colorBit)
    {
        // Read the background palette register (assuming 0xFF47 is the BG palette register address)
        uint8_t palette = mmu.readRam(0xFF47);

        // Extract the color from the palette using the colorBit
        uint8_t color = (palette >> (colorBit * 2)) & 0x03;

        // Convert the color to an ARGB value
        switch (color)
        {
        case 0:
            return 0xFFFFFFFF; // White
        case 1:
            return 0xFFC0C0C0; // Light Gray
        case 2:
            return 0xFF606060; // Dark Gray
        case 3:
            return 0xFF000000; // Black
        }

        // Default case (shouldn't be reached)
        return 0xFFFFFFFF;
    }

}