#include "gbException.h"
#include "gameboy.h"
#include "logger.h"

namespace gasyboy
{
    GameBoy::GameBoy(const std::string &filePath, const bool &bootBios)
        : _gamepad(),
          _mmu(filePath, _gamepad),
          _registers(_mmu),
          _interruptManager(_mmu, _registers),
          _cpu(bootBios, _mmu, _registers, _interruptManager),
          _gpu(_mmu),
          _timer(_mmu, _interruptManager),
          _cycleCounter(0)
    {
        // Initializing SDL App
        try
        {
            if (SDL_Init(SDL_INIT_VIDEO))
            {
                throw exception::GbException("Error when initializing SDL App.");
                exit(ExitState::CRITICAL_ERROR);
            }
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
        }
    }

    GameBoy::~GameBoy()
    {
        SDL_DestroyWindow(_window);
        SDL_Quit();
    }

    void GameBoy::step()
    {
        const int cycle = _cpu.step();
        _cycleCounter += cycle;
        _timer.updateTimer(cycle);
        _gpu.step(cycle);
        _gamepad.handleEvent();
        _interruptManager.handleInterrupts();
    }

    void GameBoy::boot()
    {
        bool exit = false;
        int fpsCounter = 0;
        uint32_t fps = SDL_GetTicks();

        try
        {
            while (!exit)
            {
                _cycleCounter = 0;
                int firstTime = SDL_GetTicks();

                while (_cycleCounter <= 69905)
                {
                    step();
                    _gpu.render();
                }

                // setting main palette
                if (_gamepad.getChangePalette())
                {
                    _gpu.changeMainPalette();
                    _gamepad.setChangePalette(false);
                }

                int elapsedTime = SDL_GetTicks() - firstTime;

                if (elapsedTime < 1000 / FPS)
                    SDL_Delay(1000 / FPS - elapsedTime);

                if (SDL_GetTicks() - fps >= 1000)
                {
                    fps = SDL_GetTicks();
                    fpsCounter = 0;
                }
                else
                {
                    fpsCounter++;
                }
            }
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
        }
    }
}
