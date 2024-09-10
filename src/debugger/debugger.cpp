#include "debugger.h"
#include "gameboy.h"
#include <chrono>

namespace gasyboy
{

    Debugger::Debugger(Registers &registers)
        : _registers(registers),
          _window(nullptr),
          _renderer(nullptr)
    {
        // Initialize SDL and ImGui in the new thread
        SDL_Init(SDL_INIT_VIDEO);
        _window = SDL_CreateWindow("Debugger", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
        _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();

        ImGui_ImplSDL2_InitForSDLRenderer(_window, _renderer);
        ImGui_ImplSDLRenderer2_Init(_renderer);
    }

    Debugger::~Debugger()
    {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(_renderer);
        SDL_DestroyWindow(_window);
        SDL_Quit();
    }

    void Debugger::render()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
        }

        // Start ImGui frame
        ImGui_ImplSDL2_NewFrame();
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui::NewFrame();

        // Render ImGui components
        ImGui::Begin("Registers", nullptr, ImGuiWindowFlags_NoMove);

        ImGui::Text("Registers:");
        ImGui::Text("AF: 0x%04X", _registers.AF.get());
        ImGui::Text("BC: 0x%04X", _registers.BC.get());
        ImGui::Text("DE: 0x%04X", _registers.DE.get());
        ImGui::Text("HL: 0x%04X", _registers.HL.get());
        ImGui::Text("SP: 0x%04X", _registers.SP);
        ImGui::Text("PC: 0x%04X", _registers.PC);

        if (ImGui::Button("Pause", ImVec2(75, 0)))
        {
            std::cout << "Pause pressed!\n";
            GameBoy::state = GameBoy::State::PAUSED;
        }

        ImGui::SameLine();
        if (ImGui::Button("Run", ImVec2(75, 0)))
        {
            std::cout << "Resume pressed!\n";
            GameBoy::state = GameBoy::State::RUNNING;
        }

        ImGui::End();

        ImGui::Render();
        SDL_RenderClear(_renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _renderer);
        SDL_RenderPresent(_renderer);
    }
}