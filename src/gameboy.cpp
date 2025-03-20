#include "interruptManagerProvider.h"
#include "utilitiesProvider.h"
#include "registersProvider.h"
#include "gamepadProvider.h"
#include "timerProvider.h"
#include "ppuProvider.h"
#include "cpuProvider.h"
#include "mmuProvider.h"
#include "gbException.h"
#include "gameboy.h"
#include "logger.h"
#ifndef EMSCRIPTEN
#include "imgui_impl_sdl2.h"
#endif

namespace gasyboy
{
    GameBoy::State GameBoy::state = GameBoy::State::RUNNING;

    GameBoy::GameBoy(const bool &wasReset)
        : _debugMode(provider::UtilitiesProvider::getInstance()->debugMode),
          _gamepad(provider::GamepadProvider::getInstance()),
          _mmu(provider::MmuProvider::getInstance()),
          _registers(provider::RegistersProvider::getInstance()),
          _interruptManager(provider::InterruptManagerProvider::getInstance()),
          _cpu(provider::CpuProvider::getInstance()),
          _timer(provider::TimerProvider::getInstance()),
          _cycleCounter(0),
          _ppu(provider::PpuProvider::getInstance())
    {
        if (!wasReset)
        {
            _renderer = std::make_unique<Renderer>();
            _renderer->init();
        }

#ifndef EMSCRIPTEN
        if (_debugMode)
        {
            _debugger = std::make_unique<Debugger>(_renderer->_window);
        }
#endif
    }

    GameBoy::GameBoy(const uint8_t *bytes, const size_t &romSize, const bool &wasReset)
        : _debugMode(provider::UtilitiesProvider::getInstance()->debugMode),
          _gamepad(provider::GamepadProvider::getInstance()),
          _mmu(provider::MmuProvider::create(bytes, romSize)),
          _registers(provider::RegistersProvider::getInstance()),
          _interruptManager(provider::InterruptManagerProvider::getInstance()),
          _cpu(provider::CpuProvider::getInstance()),
          _timer(provider::TimerProvider::getInstance()),
          _cycleCounter(0),
          _ppu(provider::PpuProvider::getInstance())
    {
        if (wasReset)
        {
            _renderer = std::make_unique<Renderer>();
            _renderer->init();
        }

#ifndef EMSCRIPTEN
        if (_debugMode)
        {
            _debugger = std::make_unique<Debugger>(_renderer->_window);
        }
#endif
    }

    void GameBoy::step()
    {
        const uint16_t cycle = static_cast<uint16_t>(_cpu->step());
        _cycleCounter += cycle;
        _timer->update(cycle);
        _gamepad->handleEvent();
        _interruptManager->handleInterrupts();
        _ppu->step(cycle);
    }

    void GameBoy::stop()
    {
        state = State::STOPPED;
    }

#ifndef EMSCRIPTEN
    void GameBoy::setDebugMode(const bool &debugMode)
    {
        _debugMode = debugMode;
        if (debugMode)
        {
            _debugger = std::make_unique<Debugger>(_renderer->_window);
        }
    }
#endif

    void GameBoy::boot()
    {
        bool running = true;
        try
        {
            while (running)
            {
#ifndef EMSCRIPTEN
                if (_debugMode)
                {
                    _debugger->render();
                }
#endif

                loop();
            }
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL, e.what());
        }
    }

    void GameBoy::loop()
    {
        _cycleCounter = 0;

        SDL_Event event;

        while ((Cpu::state == Cpu::State::RUNNING && _cycleCounter <= MAXCYCLE) ||
               Cpu::state == Cpu::State::STEPPING)
        {
            while (SDL_PollEvent(&event) != 0)
            {
                if (event.type == SDL_QUIT)
                {
                    exit(ExitState::MANUAL_STOP);
                }
                else if (event.type == SDL_DROPFILE)
                {
                    // Resetting gambeoy instance and loading new ROM
                    char *droppedFile = event.drop.file;
                    if (droppedFile)
                    {
                        gasyboy::provider::GamepadProvider::deleteInstance();
                        gasyboy::provider::MmuProvider::deleteInstance();
                        gasyboy::provider::RegistersProvider::deleteInstance();
                        gasyboy::provider::InterruptManagerProvider::deleteInstance();
                        gasyboy::provider::CpuProvider::deleteInstance();
                        gasyboy::provider::TimerProvider::deleteInstance();
                        gasyboy::provider::PpuProvider::deleteInstance();

                        std::string filePath(droppedFile);
                        gasyboy::provider::UtilitiesProvider::getInstance()->romFilePath = filePath;
                        gasyboy::provider::GamepadProvider::getInstance()->reset();
                        gasyboy::provider::MmuProvider::getInstance()->reset();
                        gasyboy::provider::RegistersProvider::getInstance()->reset();
                        gasyboy::provider::InterruptManagerProvider::getInstance()->reset();
                        gasyboy::provider::CpuProvider::getInstance()->reset();
                        gasyboy::provider::TimerProvider::getInstance()->reset();
                        gasyboy::provider::PpuProvider::getInstance()->reset();

                        _debugMode = provider::UtilitiesProvider::getInstance()->debugMode;
                        _gamepad = provider::GamepadProvider::getInstance();
                        _mmu = provider::MmuProvider::getInstance();
                        _registers = provider::RegistersProvider::getInstance();
                        _interruptManager = provider::InterruptManagerProvider::getInstance();
                        _cpu = provider::CpuProvider::getInstance();
                        _timer = provider::TimerProvider::getInstance();
                        _cycleCounter = 0;
                        _ppu = provider::PpuProvider::getInstance();
                        _cpu->state = Cpu::State::RUNNING;
                        _cycleCounter = 0;
                        _renderer->reset();

                        SDL_free(droppedFile);
                    }
                }
            }

            step();
            if (Cpu::state == Cpu::State::STEPPING)
            {
                Cpu::state = Cpu::State::PAUSED;
            }
        }

        if (_ppu->_canRender)
        {
            _renderer->render();
            _ppu->_canRender = false;
        }
    }

    void GameBoy::reset()
    {
        gasyboy::provider::GamepadProvider::deleteInstance();
        gasyboy::provider::MmuProvider::deleteInstance();
        gasyboy::provider::RegistersProvider::deleteInstance();
        gasyboy::provider::InterruptManagerProvider::deleteInstance();
        gasyboy::provider::CpuProvider::deleteInstance();
        gasyboy::provider::TimerProvider::deleteInstance();
        gasyboy::provider::PpuProvider::deleteInstance();

        gasyboy::provider::GamepadProvider::getInstance()->reset();
        gasyboy::provider::MmuProvider::getInstance()->reset();
        gasyboy::provider::RegistersProvider::getInstance()->reset();
        gasyboy::provider::InterruptManagerProvider::getInstance()->reset();
        gasyboy::provider::CpuProvider::getInstance()->reset();
        gasyboy::provider::TimerProvider::getInstance()->reset();
        gasyboy::provider::PpuProvider::getInstance()->reset();
        _debugMode = provider::UtilitiesProvider::getInstance()->debugMode;
        _gamepad = provider::GamepadProvider::getInstance();
        _mmu = provider::MmuProvider::getInstance();
        _registers = provider::RegistersProvider::getInstance();
        _interruptManager = provider::InterruptManagerProvider::getInstance();
        _cpu = provider::CpuProvider::getInstance();
        _timer = provider::TimerProvider::getInstance();
        _cycleCounter = 0;
        _ppu = provider::PpuProvider::getInstance();
    }
}
