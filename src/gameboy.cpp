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
          _timer(_mmu, _interruptManager),
          _cycleCounter(0),
          _ppu(_mmu)
    {
    }

    GameBoy::~GameBoy()
    {
    }

    void GameBoy::step()
    {
        const int cycle = _cpu.step();
        _cycleCounter += cycle;
        _ppu.step(cycle);
        _timer.updateTimer(cycle);
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
                }

                // _ppu.render();

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
