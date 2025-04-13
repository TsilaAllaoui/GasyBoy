#include "interruptManagerProvider.h"
#include "registersProvider.h"
#include "cpuProvider.h"
#include "mmuProvider.h"
#include "ppuProvider.h"
#include "renderer.h"

#ifndef EMSCRIPTEN
#include <SDL_syswm.h>
#endif

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

    Renderer::~Renderer()
    {
        // Destroy viewport texture if allocated
        if (_viewportTexture)
        {
            SDL_DestroyTexture(_viewportTexture);
            _viewportTexture = nullptr;
        }

        // Destroy renderer if allocated
        if (_renderer)
        {
            SDL_DestroyRenderer(_renderer);
            _renderer = nullptr;
        }

        // Destroy window if allocated
        if (_window)
        {
            SDL_DestroyWindow(_window);
            _window = nullptr;
        }
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

#ifndef EMSCRIPTEN
        // Set window always on top
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);

        if (SDL_GetWindowWMInfo(_window, &wmInfo))
        {
#ifdef _WIN32
            HWND hwnd = wmInfo.info.win.window;

            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE);
#endif
        }
        else
        {
            SDL_Log("Could not get window information: %s", SDL_GetError());
        }
#endif
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

        // FPS Calculation and Update
        _frameCount++;
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - _fpsTimerStart).count();

        if (elapsed >= 1000) // Update every 1 second
        {
            std::string title = "GasyBoy - FPS: " + std::to_string(_frameCount);
            SDL_SetWindowTitle(_window, title.c_str());

            _frameCount = 0;
            _fpsTimerStart = currentTime;
        }
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

    void Renderer::reset()
    {
        // Reset viewport settings
        _viewportWidth = 160;
        _viewportHeight = 144;
        _viewportRect = {0, 0, _viewportWidth, _viewportHeight};
        _viewportPixels.fill(0);

        // Reset framerate timing
        _framerateTime = 1000 / 60;
        _startFrame = std::chrono::steady_clock::now();
        _endFrame = _startFrame;

        // Clear the renderer
        if (_renderer)
        {
            SDL_RenderClear(_renderer);
        }

        // Destroy the existing texture and recreate it
        if (_viewportTexture)
        {
            SDL_DestroyTexture(_viewportTexture);
            _viewportTexture = nullptr;
        }

        // Resetting the CPU, PPU, and memory components (if needed)
        _cpu = provider::CpuProvider::getInstance();
        _ppu = provider::PpuProvider::getInstance();
        _mmu = provider::MmuProvider::getInstance();
        _interruptManager = provider::InterruptManagerProvider::getInstance();

        // Fill pixels to white
        _viewportPixels.fill(0xFF);

        // Recreate the viewport texture
        _viewportTexture = SDL_CreateTexture(_renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             _viewportWidth,
                                             _viewportHeight);
    }
}
