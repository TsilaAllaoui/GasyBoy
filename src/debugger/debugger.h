#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "SDL.h"
#include "registers.h"
#include <chrono>
#include "timer.h"
#include <functional>
#include "ppu.h"
#include "disassembler.h"
#include <thread>
#include <mutex>

namespace gasyboy
{
    class Debugger
    {
    public:
        Debugger(SDL_Window *mainWindow);
        ~Debugger();

        void render();
        void renderCpuDebugScreen();
        void renderTimerDebugScreen();
        void renderJoypadDebugScreen();
        void renderMemoryViewerDebugScreen();
        void renderPpuViewerDebugScreen();

        static std::mutex mtx;

        SDL_Window *_window;

        // Breakpoints
        std::vector<uint16_t> _breakPoints;
        int16_t _currentBreakPoint;

        int _currentSelectedRomBank;
        int _currentSelectedRamBank;

        // PPU Control Register Flags
        bool _lcdEnable;
        bool _windowTimeMapArea;
        bool _windowEnable;
        bool _objSize8x8;
        bool _objSize8x16;
        bool _objEnabled;
        bool _bgWindowEnablePriority;

        // PPU Lcd Status
        int _ly;
        int _lyc;
        int _scx;
        int _scy;
        int _wx;
        int _wy;

        // PPU Palettes
        int _bgp;
        int _obp0;
        int _obp1;

        // Preview sprite
        ImVec2 _previewPos;
        Mmu::Sprite _previewSprite;
        bool _previewSpriteXFlip;
        bool _previewSpriteYFlip;
        bool _previewSpritePriority;

        // Boot bios
        bool _executeBios;

        std::thread _disassemblerThread;

    private:
        SDL_Renderer *_renderer;
        std::shared_ptr<Registers> _registers;
        std::shared_ptr<Mmu> _mmu;
        std::shared_ptr<Timer> _timer;
        std::shared_ptr<Ppu> _ppu;
        Disassembler _disassembler;

        std::map<std::string, char *> _bytesBuffers;
        std::map<std::string, char *> _wordsBuffers;
        void renderByte(const std::string &reg, std::function<uint8_t()> get, std::function<void(uint8_t)> set);
        void renderWord(const std::string &reg, std::function<uint16_t()> get, std::function<void(const uint16_t &value)> set, const size_t &base = 16);

        std::vector<std::pair<std::string, bool>> _buttons;
        std::vector<std::pair<std::string, bool>> _directions;

        void showByteArray(const std::vector<uint8_t> &data, const uint16_t &offset = 0, size_t bytes_per_row = 16);
        void showIntegerCombo(int a, int b, int &selected_value);

        void showPalette(const char *label, Colour palette[4], const uint8_t &w = 50, const uint8_t &h = 50);
        ImU32 PixelToColor(uint8_t pixelValue);
        void RenderSprite(const Mmu::Sprite &sprite);
        void Debugger::renderPreviewSprite();
        void renderDisassemblerScreen();

        void reset();
    };
}

#endif