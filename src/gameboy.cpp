#include "gbException.h"
#include "gameboy.h"
#include "logger.h"
#include "imgui_impl_sdl2.h"

namespace gasyboy
{
    GameBoy::State GameBoy::state = GameBoy::State::RUNNING;

    GameBoy::GameBoy(const std::string &filePath, const bool &bootBios, const bool &debugMode)
        : _debugMode(debugMode),
          _gamepad(),
          _mmu(filePath, _gamepad),
          _registers(_mmu),
          _interruptManager(_mmu, _registers),
          _cpu(bootBios, _mmu, _registers, _interruptManager),
          _timer(_mmu, _interruptManager),
          _cycleCounter(0),
          _ppu(_registers, _interruptManager, _mmu)
    {

        _renderer = std::make_unique<Renderer>(_cpu, _ppu, _registers, _interruptManager, _mmu);
        _renderer->init();

        if (_debugMode)
        {
            _debugger = std::make_unique<Debugger>(_registers, _renderer->_window);
        }
    }

    void GameBoy::step()
    {
        const int cycle = _cpu.step();
        _cycleCounter += cycle;
        _timer.updateTimer(cycle);
        _gamepad.handleEvent();
        _interruptManager.handleInterrupts();
        _ppu.step(cycle);
    }

    void GameBoy::stop()
    {
        state = State::STOPPED;
    }

    void GameBoy::boot()
    {
        bool running = true;
        SDL_Event event;
        try
        {
            while (running)
            {
                if (_debugMode)
                {
                    imguiEventHandler(&event, running);
                    _debugger->render();
                }

                if (state == State::RUNNING)
                {
                    loop();
                }
            }
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
        }
    }

    void GameBoy::loop()
    {
        _cycleCounter = 0;

        while (_cycleCounter <= MAXCYCLE)
        {
            step();
        }

        if (_ppu._canRender)
        {
            _renderer->render();
            _ppu._canRender = false;
        }
    }

    void GameBoy::imguiEventHandler(SDL_Event *event, bool &running)
    {
        while (SDL_PollEvent(event))
        {
            if (event->type == SDL_QUIT)
            {
                running = false;
            }

            ImGui_ImplSDL2_ProcessEvent(event);
        }
    }
}
