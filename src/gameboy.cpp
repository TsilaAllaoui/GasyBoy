#include "gbException.h"
#include "gameboy.h"
#include "logger.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

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

        // // Init renderer
        // _renderer->init();

        // // Init renderer
        // _renderer->init();

        // // Initialize ImGui context
        // ImGui::CreateContext();

        // // Initialize ImGui for SDL2 and SDL Renderer
        // ImGui_ImplSDL2_InitForSDLRenderer(_window, _renderer->_renderer);
        // ImGui_ImplSDLRenderer2_Init(_renderer->_renderer);
    }

    GameBoy::~GameBoy()
    {
        if (_renderer)
            delete _renderer;

        // Shutdown ImGui
        // ImGui_ImplSDLRenderer2_Shutdown();
        // ImGui_ImplSDL2_Shutdown();
        // ImGui::DestroyContext();
    }

    void GameBoy::step()
    {
        // SDL_Event event;
        // while (SDL_PollEvent(&event))
        // {
        //     ImGui_ImplSDL2_ProcessEvent(&event);

        //     if (event.type == SDL_QUIT)
        //     {
        //         state = State::STOPPED;
        //     }
        // }

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

                // // Start a new ImGui frame
                // ImGui_ImplSDLRenderer2_NewFrame();
                // ImGui_ImplSDL2_NewFrame();
                // ImGui::NewFrame();

                // // Example ImGui window
                // ImGui::Begin("GasyBoy Emulator");
                // ImGui::Text("Hello, ImGui!");
                // ImGui::End();

                // // Rendering
                // ImGui::Render();
                // _renderer->render(); // Your existing rendering logic
                // ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _renderer->_renderer);
            }
        }
        catch (const exception::GbException &e)
        {
            utils::Logger::getInstance()->log(utils::Logger::LogType::CRITICAL,
                                              e.what());
        }
    }
}
