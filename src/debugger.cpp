#include "debugger.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

namespace gasyboy
{
    Debugger::Debugger()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();

        // Setup Platform/Renderer backends
        SDL_CreateWindowAndRenderer(DEBUGGER_SCREEN_WIDTH,
                                    DEBUGGER_SCREEN_HEIGHT,
                                    0,
                                    &_imguiWindow,
                                    &_imguiRenderer);

        ImGui_ImplSDL2_InitForSDLRenderer(_imguiWindow, _imguiRenderer);
        ImGui_ImplSDLRenderer2_Init(_imguiRenderer);
    }

    Debugger::~Debugger()
    {
        // Shutdown ImGui
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void Debugger::render()
    {
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow((bool *)1);

        ImGui::Render();
        SDL_RenderClear(_imguiRenderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _imguiRenderer);
        SDL_RenderPresent(_imguiRenderer);
    }
}