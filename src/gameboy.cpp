#include "gbException.h"
#include "gameboy.h"
#include "logger.h"

namespace gasyboy
{
    // Set emulator state
    GameBoy::State GameBoy::state = State::RUNNING;

    GameBoy::GameBoy(const std::string &filePath, const bool &bootBios, const bool &debugMode)
        : _gamepad(),
          _mmu(filePath, _gamepad),
          _registers(_mmu),
          _interruptManager(_mmu, _registers),
          _cpu(bootBios, _mmu, _registers, _interruptManager),
          _timer(_mmu, _interruptManager),
          _cycleCounter(0),
          _ppu(_registers, _interruptManager, _mmu)
    {
        if (debugMode)
            _renderer = new DebugRenderer(_cpu, _ppu, _registers, _interruptManager, _mmu);
        else
            _renderer = new Renderer(_cpu, _ppu, _registers, _interruptManager, _mmu);

        // Init renderer
        _renderer->init();
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

    void GameBoy::boot()
    {
        try
        {
            while (state != State::STOPPED)
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
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
        }
    }
}
