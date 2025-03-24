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
#include <thread>
#ifndef EMSCRIPTEN
#include "imgui_impl_sdl2.h"
#endif

namespace gasyboy
{
    GameBoy::State GameBoy::state = GameBoy::State::RUNNING;

    GameBoy::GameBoy()
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
        _renderer = std::make_unique<Renderer>();
        _renderer->init();

#ifndef EMSCRIPTEN
        if (_debugMode)
        {
            _debugger = std::make_shared<Debugger>(_renderer->_window);
        }
#endif
    }

    GameBoy::GameBoy(const uint8_t *bytes, const size_t &romSize)
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
        _renderer = std::make_unique<Renderer>();
        _renderer->init();

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

    void GameBoy::boot()
    {
        bool running = true;

        std::thread eventThread([&running, this]
                                {
            while (running)
            {
               _gamepad->handleEvent();
               if (provider::UtilitiesProvider::getInstance()->wasReset)
               {
                reset();
                provider::UtilitiesProvider::getInstance()->wasReset = false;
               }
            } });

        try
        {
            while (running)
            {
                loop();
            }
            eventThread.join();
        }

        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL, e.what());
        }
    }

    void GameBoy::loop()
    {
        _cycleCounter = 0;

        while ((Cpu::state == Cpu::State::RUNNING && _cycleCounter <= MAXCYCLE) ||
               Cpu::state == Cpu::State::STEPPING ||
               Cpu::state == Cpu::State::PAUSED) // Keep processing when paused
        {
            _interruptManager->handleInterrupts();

            if (Cpu::state == Cpu::State::RUNNING || Cpu::state == Cpu::State::STEPPING)
            {
                step();
                if (Cpu::state == Cpu::State::STEPPING)
                {
                    Cpu::state = Cpu::State::PAUSED;
                }
            }
            else if (Cpu::state == Cpu::State::PAUSED)
            {
                // Process SDL events while paused
                SDL_Event event;
                while (SDL_PollEvent(&event))
                {
#ifndef EMSCRIPTEN
                    if (_debugMode)
                    {
                        ImGui_ImplSDL2_ProcessEvent(&event);
                    }
#endif
                    if (event.type == SDL_QUIT)
                    {
                        GameBoy::state = GameBoy::State::STOPPED;
                        return;
                    }
                }

                // Render debugger UI while paused
#ifndef EMSCRIPTEN
                if (_debugMode)
                {
                    _debugger->render();
                }
#endif

                // Add a small delay to avoid high CPU usage
                SDL_Delay(10);
            }
        }

#ifndef EMSCRIPTEN
        if (_debugMode)
        {
            _debugger->render();
        }
#endif

        if (_ppu->_canRender)
        {
            _renderer->render();
            _ppu->_canRender = false;
        }
    }

    void GameBoy::reset()
    {
        // Saving RAM to file
        gasyboy::provider::MmuProvider::getInstance()->saveRam();

        gasyboy::provider::GamepadProvider::deleteInstance();
        gasyboy::provider::MmuProvider::deleteInstance();
        gasyboy::provider::RegistersProvider::deleteInstance();
        gasyboy::provider::InterruptManagerProvider::deleteInstance();
        gasyboy::provider::CpuProvider::deleteInstance();
        gasyboy::provider::TimerProvider::deleteInstance();
        gasyboy::provider::PpuProvider::deleteInstance();
#ifndef EMSCRIPTEN
        if (_debugMode)
        {
            _debugger->reset();
        }
        provider::UtilitiesProvider::getInstance()->romFilePath = provider::UtilitiesProvider::getInstance()->newRomFilePath;
#endif

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
    }
}
