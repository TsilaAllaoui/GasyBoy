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
#ifndef __EMSCRIPTEN__
#include "imgui_impl_sdl2.h"
#endif

namespace gasyboy
{
    GameBoy::State GameBoy::state = GameBoy::State::RUNNING;

    GameBoy::GameBoy()
        : _debugMode(provider::UtilitiesProvider::getInstance().debugMode),
          _gamepad(provider::GamepadProvider::getInstance()),
          _mmu(provider::MmuProvider::getInstance()),
          _registers(provider::RegistersProvider::getInstance()),
          _interruptManager(provider::InterruptManagerProvider::getInstance()),
          _cpu(provider::CpuProvider::getInstance()),
          _timer(provider::TimerProvider::getInstance()),
          _cycleCounter(0),
          _ppu(provider::PpuProvider::getInstance())
    {

        _renderer = std::make_unique<Renderer>(_cpu, _ppu, _registers, _interruptManager, _mmu);
        _renderer->init();

#ifndef __EMSCRIPTEN__
        if (_debugMode)
        {
            _debugger = std::make_unique<Debugger>(_renderer->_window);
        }
#endif
    }

    GameBoy::GameBoy(const uint8_t *bytes, const size_t &romSize)
        : _debugMode(provider::UtilitiesProvider::getInstance().debugMode),
          _gamepad(provider::GamepadProvider::getInstance()),
          _mmu(provider::MmuProvider::create(bytes, romSize)),
          _registers(provider::RegistersProvider::getInstance()),
          _interruptManager(provider::InterruptManagerProvider::getInstance()),
          _cpu(provider::CpuProvider::getInstance()),
          _timer(provider::TimerProvider::getInstance()),
          _cycleCounter(0),
          _ppu(provider::PpuProvider::getInstance())
    {
        _renderer = std::make_unique<Renderer>(_cpu, _ppu, _registers, _interruptManager, _mmu);
        _renderer->init();

#ifndef __EMSCRIPTEN__
        if (_debugMode)
        {
            _debugger = std::make_unique<Debugger>(_renderer->_window);
        }
#endif
    }

    void GameBoy::step()
    {
        const uint16_t cycle = static_cast<uint16_t>(_cpu.step());
        _cycleCounter += cycle;
        _timer.update(cycle);
        _gamepad.handleEvent();
        _interruptManager.handleInterrupts();
        _ppu.step(cycle);
    }

    void GameBoy::stop()
    {
        state = State::STOPPED;
    }

#ifndef __EMSCRIPTEN__
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
#ifndef __EMSCRIPTEN__
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

        while ((Cpu::state == Cpu::State::RUNNING && _cycleCounter <= MAXCYCLE) ||
               Cpu::state == Cpu::State::STEPPING)
        {
            step();
            if (Cpu::state == Cpu::State::STEPPING)
            {
                Cpu::state = Cpu::State::PAUSED;
            }
        }

        if (_ppu._canRender)
        {
            _renderer->render();
            _ppu._canRender = false;
        }
    }
}
