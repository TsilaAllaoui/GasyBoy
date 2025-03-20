#include "interruptManagerProvider.h"
#include "registersProvider.h"
#include "cpuProvider.h"
#include "mmuProvider.h"
#include "ppuProvider.h"
#include "renderer.h"

namespace gasyboy
{
    Renderer::Renderer()
        : _cpu(provider::CpuProvider::getInstance()),
          _ppu(provider::PpuProvider::getInstance()),
          _registers(provider::RegistersProvider::getInstance()),
          _interruptManager(provider::InterruptManagerProvider::getInstance()),
          _mmu(provider::MmuProvider::getInstance())
    {
    }

    void Renderer::init()
    {
        // Fill pixels to white
        _viewportPixels.fill(0xFF);

        // Iniy SDL and _window
        initWindow(_windowWidth, _windowHeight);

        // Create viewport texture
        _viewportTexture = SDL_CreateTexture(_renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             _viewportWidth,
                                             _viewportHeight);
    }

    void Renderer::initWindow(int windowWidth, int windowHeight)
    {
        SDL_Init(SDL_INIT_VIDEO);

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
        // Framerate check
        checkFramerate();

        ColorMode colorMode = ColorMode::NORMAL; // TODO: Make it dynamic

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
        SDL_SetRenderTarget(_renderer, _viewportTexture);

        // Draw on viewport
        draw();

        // Present
        SDL_RenderCopy(_renderer, _viewportTexture, NULL, &_viewportRect);
        SDL_RenderPresent(_renderer);
    }

    void Renderer::draw()
    {
        for (int i = 0; i < 144 * 160; i++)
        {
            Colour colour = _ppu->_framebuffer[i];
            std::copy(colour.colours, colour.colours + 4, _viewportPixels.begin() + i * 4);
        }
        SDL_UpdateTexture(_viewportTexture, NULL, _viewportPixels.data(), _viewportWidth * 4);
    }

    /*DEBUGGER*/

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
        for (int i = 0; i < 144 * 160; i++)
        {
            Colour colour = _ppu->_framebuffer[i];
            std::copy(colour.colours, colour.colours + 4, _viewportPixels.begin() + i * 4);
        }
        SDL_UpdateTexture(_viewportTexture, NULL, _viewportPixels.data(), _viewportWidth * 4);

        drawBackground();
        drawTilemap();
        drawSpritemap();

        SDL_RenderCopy(_renderer, backgroundTexture, NULL, &background_rect);
        SDL_RenderCopy(_renderer, tilemapTexture, NULL, &tilemapRect);
        SDL_RenderCopy(_renderer, spritemapTexture, NULL, &spritemapRect);

        drawBackgroundOverflow();
    }

    void DebugRenderer::drawBackground()
    {
        for (uint16_t i = 0; i <= 1023; i++)
        {
            uint16_t tile = _mmu->readRam(0x9800 + i);
            if (!_ppu->LCDC->bgWindowDataSelect && tile < 128)
                tile += 256;

            for (int y = 0; y < 8; y++)
            {
                for (int x = 0; x < 8; x++)
                {
                    uint8_t color = _mmu->tiles[tile].pixels[y][x];
                    int xi = (i % 32) * 8 + x;
                    int yi = (i / 32) * 8 + y;
                    int offset = 4 * (yi * backgroundWidth + xi);
                    Colour colour = _mmu->palette_BGP[color];
                    std::copy(colour.colours, colour.colours + 4, background_pixels.begin() + offset);
                }
            }
        }

        // Draw sprites
        for (auto sprite : _mmu->sprites)
        {
            if (!sprite.ready)
                continue;
            for (int tile_num = 0; tile_num < 1 + int(_ppu->LCDC->spriteSize); tile_num++)
            {
                int y_pos = sprite.y + tile_num * 8;
                // Iterate over both tiles
                for (uint8_t x = 0; x < 8; x++)
                {
                    for (uint8_t y = 0; y < 8; y++)
                    {
                        uint8_t xF = sprite.options.xFlip ? static_cast<uint8_t>(7 - x) : x;
                        uint8_t yF = sprite.options.yFlip ? static_cast<uint8_t>(7 - y) : y;

                        int tile = sprite.tile & (_ppu->LCDC->spriteSize ? 0xFE : 0xFF);
                        uint8_t colour_n = _mmu->tiles[tile + tile_num].pixels[yF][xF];

                        if (!colour_n)
                            continue;
                        int xi = (_mmu->readRam(0xff43) + sprite.x + x) % 256;
                        int yi = (_mmu->readRam(0xff42) + y_pos + y) % 256;
                        int offset = 4 * (yi * backgroundWidth + xi);

                        if (offset >= background_pixels.size())
                            continue;
                        Colour colour = sprite.colourPalette[colour_n];
                        std::copy(colour.colours, colour.colours + 4, background_pixels.begin() + offset);
                    }
                }
            }
        }
        SDL_UpdateTexture(backgroundTexture, NULL, background_pixels.data(), backgroundWidth * 4);
    }

    void DebugRenderer::drawTilemap()
    {
        for (int i = 0; i < 384; i++)
        {
            for (int y = 0; y < 8; y++)
            {
                for (int x = 0; x < 8; x++)
                {
                    uint8_t colour_n = _mmu->tiles[i].pixels[y][x];
                    int offsetX = ((i * 8 + x) % tilemapWidth);
                    int offsetY = y + (int(i / 16)) * 8;
                    int offset = 4 * (offsetY * tilemapWidth + offsetX);

                    Colour colour = _mmu->palette_BGP[colour_n];
                    std::copy(colour.colours, colour.colours + 4, tilemap_pixels.begin() + offset);
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
        int overflowX = std::max(*_ppu->SCX + _viewportWidth - backgroundWidth, 0);
        int overflowY = std::max(*_ppu->SCY + _viewportHeight - backgroundHeight, 0);

        drawRectangle(background_rect.x + *_ppu->SCX, _viewportHeight + *_ppu->SCY, _viewportWidth - overflowX, _viewportHeight, {255, 255, 255, 100});

        if (overflowX)
            drawRectangle(background_rect.x, _viewportHeight + *_ppu->SCY, overflowX, _viewportHeight, {255, 255, 255, 100});

        if (overflowY)
            drawRectangle(background_rect.x + *_ppu->SCX, _viewportHeight, _viewportWidth - overflowX, overflowY, {255, 255, 255, 100});

        if (overflowX && overflowY)
            drawRectangle(background_rect.x, _viewportHeight, overflowX, overflowY, {255, 255, 255, 100});
    }

    void DebugRenderer::drawSpritemap()
    {
        auto draw_sprite = [this](Mmu::Sprite sprite, int tile_off, int off_x, int off_y)
        {
            if (!sprite.ready)
                return;
            for (uint8_t x = 0; x < 8; x++)
            {
                uint8_t xF = sprite.options.xFlip ? static_cast<uint8_t>(7 - x) : x;
                for (uint8_t y = 0; y < 8; y++)
                {
                    uint8_t yF = sprite.options.yFlip ? static_cast<uint8_t>(7 - y) : y;
                    uint8_t colour_n = _mmu->tiles[sprite.tile + tile_off].pixels[yF][xF];
                    int offsetX = ((off_x + x) % spritemapWidth);
                    int offsetY = y + off_y;
                    int offset = 4 * (offsetY * spritemapWidth + offsetX);

                    Colour colour = sprite.colourPalette[colour_n];
                    std::copy(colour.colours, colour.colours + 4, spritemap_pixels.begin() + offset);
                }
            }
        };

        if (_ppu->LCDC->spriteSize)
        {
            for (int i = 0, row = 0; i < 20; i++)
            {
                draw_sprite(_mmu->sprites[i], 0, i * 8, row * 8);
                draw_sprite(_mmu->sprites[i], 1, i * 8, row * 8 + 8);
                if (((i + 1) % 5) == 0)
                    row += 2;
            }
        }
        else
        {
            for (int i = 0, row = 0; i < 40; i++)
            {
                draw_sprite(_mmu->sprites[i], 0, i * 8, row * 8);
                if (((i + 1) % 5) == 0)
                    row++;
            }
        }

        SDL_UpdateTexture(spritemapTexture, NULL, spritemap_pixels.data(), spritemapWidth * 4);
    }
}
