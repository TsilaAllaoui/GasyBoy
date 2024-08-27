#include "debugger.h"
// #include "imgui.h"
// #include "backends/imgui_impl_sdl2.h"
// #include "backends/imgui_impl_sdlrenderer2.h"
#include <vector>

namespace gasyboy
{
    // Function to create a Gameboy icon as an SDL_Surface
    SDL_Surface *createGameboyIcon(int width, int height)
    {
        // Create an SDL_Surface with the specified width and height
        SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

        if (!surface)
        {
            // Handle error
            return nullptr;
        }

        // Access surface pixels
        Uint32 *pixels = (Uint32 *)surface->pixels;

        // Define some colors
        Uint32 black = SDL_MapRGB(surface->format, 0, 0, 0);
        Uint32 white = SDL_MapRGB(surface->format, 255, 255, 255);
        Uint32 grey = SDL_MapRGB(surface->format, 192, 192, 192);

        // Fill the surface with a transparent background
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));

        // Draw a simple Gameboy shape
        // Outline of the Gameboy
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                if ((x >= 5 && x <= 26 && y >= 5 && y <= 27) ||
                    (x >= 3 && x <= 28 && y >= 3 && y <= 5) ||
                    (x >= 3 && x <= 5 && y >= 5 && y <= 27) ||
                    (x >= 26 && x <= 28 && y >= 5 && y <= 27))
                {
                    pixels[y * width + x] = black;
                }
            }
        }

        // Draw the screen
        for (int y = 10; y < 20; y++)
        {
            for (int x = 10; x < 22; x++)
            {
                pixels[y * width + x] = grey;
            }
        }

        // Draw buttons
        for (int y = 20; y <= 23; y++)
        {
            for (int x = 18; x <= 21; x++)
            {
                pixels[y * width + x] = black;
            }
        }

        return surface;
    }

    Debugger::Debugger(Registers &registers)
        : _registers(registers)
    {

        // // Setup Dear ImGui context
        // IMGUI_CHECKVERSION();
        // ImGui::CreateContext();
        // ImGuiIO &io = ImGui::GetIO();

        // // Setup Platform/Renderer backends
        // SDL_CreateWindowAndRenderer(DEBUGGER_SCREEN_WIDTH,
        //                             DEBUGGER_SCREEN_HEIGHT,
        //                             0,
        //                             &_imguiWindow,
        //                             &_imguiRenderer);
        // SDL_SetWindowTitle(_imguiWindow, "Debugger");

        // ImGui_ImplSDL2_InitForSDLRenderer(_imguiWindow, _imguiRenderer);
        // ImGui_ImplSDLRenderer2_Init(_imguiRenderer);
    }

    Debugger::~Debugger()
    {
        // // Shutdown ImGui
        // ImGui_ImplSDLRenderer2_Shutdown();
        // ImGui_ImplSDL2_Shutdown();
        // ImGui::DestroyContext();
    }

    void Debugger::render()
    {
        // // Start ImGui frame
        // ImGui_ImplSDL2_NewFrame();
        // ImGui_ImplSDLRenderer2_NewFrame();
        // ImGui::NewFrame();

        // ImGui::SetNextWindowPos(ImVec2(0, 0));
        // ImGui::SetNextWindowSize(ImVec2(REGISTERS_WINDOW_WIDTH, REGISTERS_WINDOW_HEIGHT));

        // ImGui::Begin("Registers", nullptr, ImGuiWindowFlags_NoMove);

        // // Show CPU registers (Assume _gameboy has access to the registers)
        // ImGui::Text("Registers:");
        // ImGui::Text("AF: %04X", _registers.AF.get());
        // ImGui::Text("BC: %04X", _registers.BC.get());
        // ImGui::Text("DE: %04X", _registers.DE.get());
        // ImGui::Text("HL: %04X", _registers.HL.get());
        // ImGui::Text("SP: %04X", _registers.SP);
        // ImGui::Text("PC: %04X", _registers.PC);

        // // First button
        // if (ImGui::Button("Pause", ImVec2(75, 0)))
        // {
        //     // Button 1 action
        // }

        // // Second button
        // ImGui::SameLine();
        // if (ImGui::Button("Run", ImVec2(75, 0)))
        // {
        //     // Button 2 action
        // }

        // ImGui::End();

        // // Render ImGui and emulator display
        // ImGui::Render();
        // SDL_RenderClear(_imguiRenderer);
        // ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _imguiRenderer);
        // SDL_RenderPresent(_imguiRenderer);
    }
}