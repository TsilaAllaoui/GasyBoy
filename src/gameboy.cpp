#include "gbException.h"
#include "gameboy.h"
#include "logger.h"

namespace gasyboy
{
    // Set emulator state
    GameBoy::State GameBoy::state = State::RUNNING;

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
        // Init renderer
        if (_debugMode)
        {
            _renderer = new DebugRenderer(_cpu, _ppu, _registers, _interruptManager, _mmu);
        }
        else
        {
            _renderer = new Renderer(_cpu, _ppu, _registers, _interruptManager, _mmu);
        }
        _renderer->init();
    }

    GameBoy::GameBoy(const uint8_t *bytes, const size_t &romSize, const bool &bootBios, const bool &debugMode)
        : _debugMode(debugMode),
          _gamepad(),
          _mmu(bytes, romSize, _gamepad),
          _registers(_mmu),
          _interruptManager(_mmu, _registers),
          _cpu(bootBios, _mmu, _registers, _interruptManager),
          _timer(_mmu, _interruptManager),
          _cycleCounter(0),
          _ppu(_registers, _interruptManager, _mmu)
    {
    }

    GameBoy::~GameBoy()
    {
        if (_renderer)
            delete _renderer;
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
        try
        {
            bool demo = true;
            while (state != State::STOPPED)
            {
                loop();
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
}
