#include "renderer.h"
#include <algorithm>
#include <thread>
#include <chrono>

namespace gasyboy
{

    Renderer::Renderer(Cpu &cpu,
                       Ppu &ppu,
                       Registers &registers,
                       InterruptManager &interruptManager,
                       Mmu &mmu)
        : _cpu(cpu),
          _ppu(ppu),
          _registers(registers),
          _interruptManager(interruptManager),
          _mmu(mmu)
    {
    }

    void Renderer::init()
    {
        // Fill viewport pixel buffer with white (0xFF for each byte).
        _viewportPixels.fill(0xFF);

        // Initialize SDL window and renderer.
        initWindow(_windowWidth, _windowHeight);

        // Create viewport texture in ARGB8888 format.
        _viewportTexture = SDL_CreateTexture(_renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             _viewportWidth,
                                             _viewportHeight);
    }

    void Renderer::initWindow(int windowWidth, int windowHeight)
    {
        SDL_Init(SDL_INIT_VIDEO);
        // Create window & renderer at double resolution.
        SDL_CreateWindowAndRenderer(windowWidth * 2, windowHeight * 2, 0, &_window, &_renderer);
        SDL_SetWindowPosition(_window, 20, 50);
        SDL_RenderSetLogicalSize(_renderer, windowWidth, windowHeight);
        SDL_SetWindowResizable(_window, SDL_TRUE);
        SDL_SetWindowTitle(_window, "GasyBoy");
    }

    void Renderer::checkFramerate()
    {
        _endFrame = std::chrono::steady_clock::now();
        auto timeTook = std::chrono::duration_cast<std::chrono::milliseconds>(_endFrame - _startFrame).count();

        if (timeTook < _framerateTime)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(_framerateTime - timeTook));
        }
        _startFrame = std::chrono::steady_clock::now();
    }

    void Renderer::render()
    {
        // Maintain steady framerate.
        checkFramerate();

        // Choose colour mode (could be made dynamic).
        ColorMode colorMode = ColorMode::NORMAL;
        switch (colorMode)
        {
        case ColorMode::NORMAL:
            SDL_SetTextureColorMod(_viewportTexture, 255, 255, 255);
            break;
        case ColorMode::RETRO:
            SDL_SetTextureColorMod(_viewportTexture, 155, 188, 15);
            break;
        case ColorMode::GREY:
            SDL_SetTextureColorMod(_viewportTexture, 224, 219, 205);
            break;
        }

        SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
        SDL_RenderClear(_renderer);

        // Set the render target to our viewport texture.
        SDL_SetRenderTarget(_renderer, _viewportTexture);

        // Draw the viewport (copying _ppu._framebuffer to our pixel buffer).
        draw();

        // Reset render target to default, then copy the viewport texture to screen.
        SDL_SetRenderTarget(_renderer, nullptr);
        SDL_RenderCopy(_renderer, _viewportTexture, NULL, &_viewportRect);
        SDL_RenderPresent(_renderer);
    }

    void Renderer::draw()
    {
        // Convert the 144x160 framebuffer (each Colour holds an array of 4 bytes)
        // into our _viewportPixels buffer (RGBA per pixel).
        for (int i = 0; i < 144 * 160; i++)
        {
            Colour colour = _ppu._framebuffer[i];
            // Copy 4 bytes from colour.colours into _viewportPixels.
            std::copy(colour.colours, colour.colours + 4, _viewportPixels.begin() + i * 4);
        }
        // Update the texture with the new pixel data.
        SDL_UpdateTexture(_viewportTexture, NULL, _viewportPixels.data(), _viewportWidth * 4);
    }

    /* --- DebugRenderer Implementation --- */

    void DebugRenderer::init()
    {
        _viewportPixels.fill(0xFF);
        tilemap_pixels.fill(0xFF);
        spritemap_pixels.fill(0xFF);
        background_pixels.fill(0xFF);

        initWindow(windowWidth, windowHeight);

        _viewportTexture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, _viewportWidth, _viewportHeight);
        backgroundTexture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, backgroundWidth, backgroundHeight);
        spritemapTexture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, spritemapWidth, spritemapHeight);
        tilemapTexture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, tilemapWidth, tilemapHeight);
    }

    void DebugRenderer::draw()
    {
        // Update the main viewport from the PPU framebuffer.
        for (int i = 0; i < 144 * 160; i++)
        {
            Colour colour = _ppu._framebuffer[i];
            std::copy(colour.colours, colour.colours + 4, _viewportPixels.begin() + i * 4);
        }
        SDL_UpdateTexture(_viewportTexture, NULL, _viewportPixels.data(), _viewportWidth * 4);

        // Draw additional debug views.
        drawBackground();
        drawTilemap();
        drawSpritemap();

        // Render textures to corresponding rectangles.
        SDL_RenderCopy(_renderer, backgroundTexture, NULL, &background_rect);
        SDL_RenderCopy(_renderer, tilemapTexture, NULL, &tilemapRect);
        SDL_RenderCopy(_renderer, spritemapTexture, NULL, &spritemapRect);

        drawBackgroundOverflow();
    }

    void DebugRenderer::drawBackground()
    {
        // Draw the entire background tile map (1024 tiles = 32x32).
        for (uint16_t i = 0; i < 1024; i++)
        {
            uint16_t tile = _mmu.readRam(0x9800 + i);
            if (!_ppu._control->bgWindowDataSelect && tile < 128)
                tile += 256;

            for (int y = 0; y < 8; y++)
            {
                for (int x = 0; x < 8; x++)
                {
                    uint8_t colorIdx = _mmu.tiles[tile].pixels[y][x];
                    int xi = (i % 32) * 8 + x;
                    int yi = (i / 32) * 8 + y;
                    int offset = 4 * (yi * backgroundWidth + xi);
                    Colour col = _mmu.palette_BGP[colorIdx];
                    std::copy(col.colours, col.colours + 4, background_pixels.begin() + offset);
                }
            }
        }

        // Overlay sprites on the background.
        for (const auto &sprite : _mmu.sprites)
        {
            if (!sprite.ready)
                continue;
            int spriteTiles = _ppu._control->spriteSize ? 2 : 1;
            for (int tile_num = 0; tile_num < spriteTiles; tile_num++)
            {
                int y_pos = sprite.y + tile_num * 8;
                for (uint8_t x = 0; x < 8; x++)
                {
                    for (uint8_t y = 0; y < 8; y++)
                    {
                        uint8_t xF = sprite.options.xFlip ? (7 - x) : x;
                        uint8_t yF = sprite.options.yFlip ? (7 - y) : y;
                        int tile = sprite.tile & (_ppu._control->spriteSize ? 0xFE : 0xFF);
                        uint8_t colorIdx = _mmu.tiles[tile + tile_num].pixels[yF][xF];
                        if (colorIdx == 0)
                            continue;
                        // Read current scroll registers only once per sprite if possible.
                        int scrollX = _mmu.readRam(0xFF43);
                        int scrollY = _mmu.readRam(0xFF42);
                        int xi = (scrollX + sprite.x + x) % backgroundWidth;
                        int yi = (scrollY + y_pos + y) % backgroundHeight;
                        int offset = 4 * (yi * backgroundWidth + xi);
                        if (offset < 0 || offset >= int(background_pixels.size()))
                            continue;
                        Colour col = _mmu.palette_BGP[colorIdx];
                        std::copy(col.colours, col.colours + 4, background_pixels.begin() + offset);
                    }
                }
            }
        }
        SDL_UpdateTexture(backgroundTexture, NULL, background_pixels.data(), backgroundWidth * 4);
    }

    void DebugRenderer::drawTilemap()
    {
        // Draw the tilemap view.
        for (int i = 0; i < 384; i++)
        {
            for (int y = 0; y < 8; y++)
            {
                for (int x = 0; x < 8; x++)
                {
                    uint8_t colorIdx = _mmu.tiles[i].pixels[y][x];
                    int offsetX = ((i * 8 + x) % tilemapWidth);
                    int offsetY = y + (i / 16) * 8;
                    int offset = 4 * (offsetY * tilemapWidth + offsetX);
                    Colour col = _mmu.palette_BGP[colorIdx];
                    std::copy(col.colours, col.colours + 4, tilemap_pixels.begin() + offset);
                }
            }
        }
        SDL_UpdateTexture(tilemapTexture, NULL, tilemap_pixels.data(), tilemapWidth * 4);
    }

    void DebugRenderer::drawRectangle(int x, int y, int width, int height, Colour color)
    {
        SDL_Rect rect = {x, y, width, height};
        SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(_renderer, &rect);
        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(_renderer, &rect);
    }

    void DebugRenderer::drawBackgroundOverflow()
    {
        int overflowX = std::max(*_ppu._scrollX + _viewportWidth - backgroundWidth, 0);
        int overflowY = std::max(*_ppu._scrollY + _viewportHeight - backgroundHeight, 0);

        drawRectangle(background_rect.x + *_ppu._scrollX, _viewportHeight + *_ppu._scrollY, _viewportWidth - overflowX, _viewportHeight, {255, 255, 255, 100});
        if (overflowX)
            drawRectangle(background_rect.x, _viewportHeight + *_ppu._scrollY, overflowX, _viewportHeight, {255, 255, 255, 100});
        if (overflowY)
            drawRectangle(background_rect.x + *_ppu._scrollX, _viewportHeight, _viewportWidth - overflowX, overflowY, {255, 255, 255, 100});
        if (overflowX && overflowY)
            drawRectangle(background_rect.x, _viewportHeight, overflowX, overflowY, {255, 255, 255, 100});
    }

    void DebugRenderer::drawSpritemap()
    {
        // Define a lambda to draw an individual sprite.
        auto draw_sprite = [this](const Mmu::Sprite &sprite, int tile_off, int off_x, int off_y)
        {
            if (!sprite.ready)
                return;
            for (uint8_t x = 0; x < 8; x++)
            {
                uint8_t xF = sprite.options.xFlip ? (7 - x) : x;
                for (uint8_t y = 0; y < 8; y++)
                {
                    uint8_t yF = sprite.options.yFlip ? (7 - y) : y;
                    uint8_t colorIdx = _mmu.tiles[sprite.tile + tile_off].pixels[yF][xF];
                    // Skip transparent pixels.
                    if (colorIdx == 0)
                        continue;
                    int offsetX = (off_x + x) % spritemapWidth;
                    int offsetY = y + off_y;
                    int offset = 4 * (offsetY * spritemapWidth + offsetX);
                    Colour col = sprite.colourPalette[colorIdx];
                    std::copy(col.colours, col.colours + 4, spritemap_pixels.begin() + offset);
                }
            }
        };

        if (_ppu._control->spriteSize)
        {
            // For 8x16 sprites, process 20 sprites (limit to 10 per scanline is not applied in debug view).
            for (int i = 0, row = 0; i < 20; i++)
            {
                draw_sprite(_mmu.sprites[i], 0, i * 8, row * 16);
                // For 8x16, draw the second tile as well.
                draw_sprite(_mmu.sprites[i], 1, i * 8, row * 16 + 8);
                if (((i + 1) % 5) == 0)
                    row += 2;
            }
        }
        else
        {
            // For 8x8 sprites, process all 40.
            for (int i = 0, row = 0; i < 40; i++)
            {
                draw_sprite(_mmu.sprites[i], 0, i * 8, row * 8);
                if (((i + 1) % 5) == 0)
                    row++;
            }
        }
        SDL_UpdateTexture(spritemapTexture, NULL, spritemap_pixels.data(), spritemapWidth * 4);
    }
}
