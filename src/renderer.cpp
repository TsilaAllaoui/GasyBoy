#include "renderer.h"

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

        SDL_CreateWindowAndRenderer(_windowWidth * 2, _windowHeight * 2, 0, &_window, &_renderer);
        SDL_RenderSetLogicalSize(_renderer, _windowWidth, _windowHeight);
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
            Colour colour = _ppu._framebuffer[i];
            std::copy(colour.colours, colour.colours + 4, _viewportPixels.begin() + i * 4);
        }
        SDL_UpdateTexture(_viewportTexture, NULL, _viewportPixels.data(), _viewportWidth * 4);
    }
}
