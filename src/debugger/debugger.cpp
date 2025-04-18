#include "ImGuiFileDialog.h"
#include "debugger.h"
#include "gameboy.h"
#include "gamepad.h"
#include "logger.h"
#include "mmu.h"
#include <chrono>

#include "interruptManagerProvider.h"
#include "registersProvider.h"
#include "utilitiesProvider.h"
#include "gamepadProvider.h"
#include "gameBoyProvider.h"
#include "timerProvider.h"
#include "mmuProvider.h"
#include "cpuProvider.h"
#include "ppuProvider.h"

namespace gasyboy
{

    Debugger::Debugger(SDL_Window *mainWindow)
        : _mmu(provider::MmuProvider::getInstance()),
          _registers(provider::RegistersProvider::getInstance()),
          _ppu(provider::PpuProvider::getInstance()),
          _window(nullptr),
          _renderer(nullptr),
          _breakPoints(),
          _currentBreakPoint(-1),
          _timer(provider::TimerProvider::getInstance()),
          _currentSelectedRomBank(1),
          _lcdEnable(false),
          _previewPos(ImVec2(845, 165)),
          _previewSprite(),
          _disassembler(),
          _executeBios(provider::UtilitiesProvider::getInstance()->executeBios)
    {
        _bytesBuffers = {
            {"A", ""},
            {"F", ""},
            {"B", ""},
            {"C", ""},
            {"D", ""},
            {"E", ""},
            {"H", ""},
            {"L", ""},
            {"DIV", ""},
            {"TIMA", ""},
            {"TMA", ""},
            {"TAC", ""},
            {"BGP", ""},
            {"OBP0", ""},
            {"OBP1", ""},
            {"X", ""},
            {"Y", ""},
            {"Tile", ""},
            {"Palette", ""},
        };

        _wordsBuffers = {
            {"SP", ""},
            {"PC", ""},
            {"TIMA_INCREMENT_RATE", ""},
            {"WINDOW_TILE_MAP_AREA", ""},
            {"BG_WINDOW_TILE_DATA_MAP_AREA", ""},
        };

        _buttons = {
            {"A", false},
            {"B", false},
            {"SELECT", false},
            {"START", false},
        };

        _directions = {
            {"RIGHT", false},
            {"LEFT", false},
            {"UP", false},
            {"DOWN", false},
        };

        // Retrieve the position and size of the main window
        int mainWindowX, mainWindowY, mainWindowWidth, mainWindowHeight;
        SDL_GetWindowPosition(mainWindow, &mainWindowX, &mainWindowY);
        SDL_GetWindowSize(mainWindow, &mainWindowWidth, &mainWindowHeight);

        // Calculate position for the debugger window
        int debuggerWindowX = mainWindowX + mainWindowWidth + 10; // 10 pixels to the right
        int debuggerWindowY = mainWindowY;

        SDL_CreateWindowAndRenderer(1340, 765, SDL_WINDOW_SHOWN, &_window, &_renderer);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        static bool initialized = false;

        if (!initialized)
        {
            ImGui_ImplSDL2_InitForSDLRenderer(_window, _renderer);
            ImGui_ImplSDLRenderer2_Init(_renderer);
            initialized = true;
        }

        // Disassembling rom
        // _disassemblerThread = std::thread([&]()
        //                                   { _disassembler.disassemble(); });
        // _disassembler.disassemble();
    }

    void Debugger::reset()
    {
        _breakPoints.clear();
        _currentBreakPoint = -1;

        _currentSelectedRomBank = 0;
        _currentSelectedRamBank = 0;

        // Reset PPU Control Register Flags
        _lcdEnable = false;
        _windowTimeMapArea = false;
        _windowEnable = false;
        _objSize8x8 = true;
        _objSize8x16 = false;
        _objEnabled = false;
        _bgWindowEnablePriority = false;

        // Reset PPU LCD Status
        _ly = 0;
        _lyc = 0;
        _scx = 0;
        _scy = 0;
        _wx = 0;
        _wy = 0;

        // Reset PPU Palettes
        _bgp = 0;
        _obp0 = 0;
        _obp1 = 0;

        // Reset Preview Sprite Data
        _previewPos = ImVec2(0, 0);
        _previewSprite = Mmu::Sprite(); // Assuming Sprite has a default constructor
        _previewSpriteXFlip = false;
        _previewSpriteYFlip = false;
        _previewSpritePriority = false;

        // Reset Boot BIOS execution flag
        _executeBios = false;

        // Reset button states
        for (auto &button : _buttons)
            button.second = false;

        for (auto &direction : _directions)
            direction.second = false;

        // Reset submodules
        _mmu = provider::MmuProvider::getInstance();
        _ppu = provider::PpuProvider::getInstance();
        _registers = provider::RegistersProvider::getInstance();

        _bytesBuffers = {
            {"A", ""},
            {"F", ""},
            {"B", ""},
            {"C", ""},
            {"D", ""},
            {"E", ""},
            {"H", ""},
            {"L", ""},
            {"DIV", ""},
            {"TIMA", ""},
            {"TMA", ""},
            {"TAC", ""},
            {"BGP", ""},
            {"OBP0", ""},
            {"OBP1", ""},
            {"X", ""},
            {"Y", ""},
            {"Tile", ""},
            {"Palette", ""},
        };

        _wordsBuffers = {
            {"SP", ""},
            {"PC", ""},
            {"TIMA_INCREMENT_RATE", ""},
            {"WINDOW_TILE_MAP_AREA", ""},
            {"BG_WINDOW_TILE_DATA_MAP_AREA", ""},
        };

        _buttons = {
            {"A", false},
            {"B", false},
            {"SELECT", false},
            {"START", false},
        };

        _directions = {
            {"RIGHT", false},
            {"LEFT", false},
            {"UP", false},
            {"DOWN", false},
        };
    }

    Debugger::~Debugger()
    {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }

    void Debugger::render()
    {
        // Start ImGui frame
        ImGui_ImplSDL2_NewFrame();
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui::NewFrame();

        // Rendering cpu state
        renderCpuDebugScreen();

        // Rendering timer state
        renderTimerDebugScreen();

        // Rendering joypad state
        renderJoypadDebugScreen();

        // Rendering Memory state
        renderMemoryViewerDebugScreen();

        // Rendering PPU state
        renderPpuViewerDebugScreen();

        // Rendering Disassembler
        // renderDisassemblerScreen();

        ImGui::Render();
        SDL_RenderClear(_renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _renderer);
        SDL_RenderPresent(_renderer);
    }

    void Debugger::renderByte(const std::string &reg, std::function<uint8_t()> get, std::function<void(uint8_t)> set)
    {
        snprintf(const_cast<char *>(_bytesBuffers[reg].c_str()), sizeof(_bytesBuffers[reg]) + 2, "0x%02X", get());
        std::string regStr = reg;
        regStr += ": ";
        ImGui::Text("%s", regStr.c_str());
        ImGui::SameLine();
        ImGui::SetNextItemWidth(50.0f);
        std::string label = "##" + reg;
        if (ImGui::InputText(label.c_str(), const_cast<char *>(_bytesBuffers[reg].c_str()), sizeof(_bytesBuffers[reg]), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            auto newValue = static_cast<uint8_t>(std::stoi(_bytesBuffers[reg], nullptr, 16));
            set(newValue);
        }
    }

    void Debugger::renderWord(const std::string &reg, std::function<uint16_t()> get, std::function<void(const uint16_t &value)> set, const size_t &base)
    {
        snprintf(const_cast<char *>(_wordsBuffers[reg].c_str()), sizeof(_wordsBuffers[reg]) + 2, base == 16 ? "0x%04X" : "%d", get());
        ImGui::Text("%s : ", reg.c_str());
        ImGui::SameLine();
        ImGui::SetNextItemWidth(50.0f);
        if (ImGui::InputText(("##" + reg).c_str(), const_cast<char *>(_wordsBuffers[reg].c_str()), sizeof(_wordsBuffers[reg]), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            auto newValue = static_cast<uint16_t>(std::stoi(_wordsBuffers[reg], nullptr, 16));
            set(newValue);
        }
    }

    void Debugger::renderCpuDebugScreen()
    {

        // Set window position and size
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(350, 235), ImGuiCond_Always);

        // Create the window
        ImGui::Begin("CPU", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        // Render registers
        if (ImGui::BeginTable("##CPU", 2, ImGuiTableFlags_Borders)) // | ImGuiWindowFlags_NoMove))
        {
            ImGui::TableSetupColumn("Registers", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Flags", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();
            renderByte("A", [&]()
                       { return _registers->AF.getLeftRegister(); }, [&](const uint8_t &val)
                       { _registers->AF.setLeftRegister(val); });
            ImGui::SameLine();
            renderByte("F", [&]()
                       { return _registers->AF.getRightRegister(); }, [&](const uint8_t &val)
                       { _registers->AF.setRightRegister(val); });

            renderByte("B", [&]()
                       { return _registers->BC.getLeftRegister(); }, [&](const uint8_t &val)
                       { _registers->BC.setLeftRegister(val); });
            ImGui::SameLine();
            renderByte("C", [&]()
                       { return _registers->BC.getRightRegister(); }, [&](const uint8_t &val)
                       { _registers->BC.setRightRegister(val); });

            renderByte("D", [&]()
                       { return _registers->DE.getLeftRegister(); }, [&](const uint8_t &val)
                       { _registers->DE.setLeftRegister(val); });
            ImGui::SameLine();
            renderByte("E", [&]()
                       { return _registers->DE.getRightRegister(); }, [&](const uint8_t &val)
                       { _registers->DE.setRightRegister(val); });

            renderByte("H", [&]()
                       { return _registers->HL.getLeftRegister(); }, [&](const uint8_t &val)
                       { _registers->HL.setLeftRegister(val); });
            ImGui::SameLine();
            renderByte("L", [&]()
                       { return _registers->HL.getRightRegister(); }, [&](const uint8_t &val)
                       { _registers->HL.setRightRegister(val); });

            renderWord("SP", [&]()
                       { return _registers->SP; }, [&](uint16_t val)
                       { _registers->SP = val; });

            renderWord("PC", [&]()
                       { return _registers->PC; }, [&](uint16_t val)
                       { _registers->PC = val; });

            ImGui::TableNextColumn();

            ImGui::Text("Flags");
            bool Z = _registers->AF.getFlag(Register::FlagName::Z);
            bool N = _registers->AF.getFlag(Register::FlagName::N);
            bool H = _registers->AF.getFlag(Register::FlagName::H);
            bool C = _registers->AF.getFlag(Register::FlagName::C);
            ImGui::Checkbox("Zero", &Z);
            ImGui::Checkbox("Subtract", &N);
            ImGui::Checkbox("Half Carry", &H);
            ImGui::Checkbox("Carry", &C);

            ImGui::Separator();
            ImGui::Text("State");
            bool halted = _registers->getHalted();
            ImGui::Checkbox("HALTED", &halted);
            _registers->setHalted(halted);
            ImGui::SameLine();
            bool stopped = _registers->getStopMode();
            ImGui::Checkbox("STOPPED", &stopped);
            _registers->setStopMode(stopped);

            ImGui::EndTable();
        }

        // Render actions
        ImGui::Text("Actions");
        if (ImGui::Button("Pause", ImVec2(75, 0)))
        {
            std::cout << "Pause pressed!\n";
            Cpu::state = Cpu::State::PAUSED;
        }

        ImGui::SameLine();
        if (ImGui::Button("Run", ImVec2(75, 0)))
        {
            std::cout << "Resume pressed!\n";
            Cpu::state = Cpu::State::RUNNING;
        }

        ImGui::SameLine();
        if (ImGui::Button("Step", ImVec2(75, 0)))
        {
            std::cout << "Step pressed!\n";
            Cpu::state = Cpu::State::STEPPING;
        }

        ImGui::SameLine();
        if (ImGui::Button("Open"))
        {
            Cpu::state = Cpu::State::PAUSED;

            IGFD::FileDialogConfig config;
            config.path = ".";
            config.flags = ImGuiFileDialogFlags_DisableCreateDirectoryButton | ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering;

            // Open the file dialog
            ImGuiFileDialog::Instance()->OpenDialog("ChooseRom", "Choose Rom File", ".gb", config);

            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(350, 235), ImGuiCond_Always);
        }

        // Inside the main loop, make sure this runs every frame
        if (ImGuiFileDialog::Instance()->Display("ChooseRom"))
        {
            if (ImGuiFileDialog::Instance()->IsOk()) // Check if the user selected a file
            {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName(); // Get selected file

                provider::UtilitiesProvider::getInstance()->romFilePath = filePath;
                provider::UtilitiesProvider::getInstance()->newRomFilePath = filePath;
                provider::GameBoyProvider::getInstance()->reset();
            }

            // Close the dialog to prevent reopening
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::End();
    }

    void Debugger::renderTimerDebugScreen()
    {
        // Set window position and size
        ImGui::SetNextWindowPos(ImVec2(0, 235), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(350, 130), ImGuiCond_Always);

        // Create the window
        ImGui::Begin("Timer", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        // Render registers
        if (ImGui::BeginTable("##Timer", 1, ImGuiTableFlags_Borders)) // | ImGuiWindowFlags_NoMove))
        {
            ImGui::TableSetupColumn("Timer", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();

            renderByte("DIV", [&]()
                       { return _timer->DIV(); }, [&](const uint8_t &value)
                       { _timer->setDIV(value); });

            renderByte("TIMA", [&]()
                       { return _timer->TIMA(); }, [&](const uint8_t &value)
                       { _timer->setTIMA(value); });
            ImGui::SameLine();
            renderWord("TIMA_INCREMENT_RATE", [&]()
                       { return _timer->_timaIncrementRate; }, [&](const uint16_t &value)
                       { _timer->_timaIncrementRate = value; }, 10);

            renderByte("TMA", [&]()
                       { return _timer->TMA(); }, [&](const uint8_t &value)
                       { _timer->setTMA(value); });

            renderByte("TAC", [&]()
                       { return _timer->TAC(); }, [&](const uint8_t &value)
                       { _timer->setTAC(value); });

            ImGui::EndTable();
        }

        ImGui::End();
    }

    void Debugger::renderJoypadDebugScreen()
    {
        // Set window position and size
        ImGui::SetNextWindowPos(ImVec2(670, 365), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(350, 130), ImGuiCond_Always);

        // Create the window
        ImGui::Begin("Joypad", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        if (!Gamepad::isButtonSelected())
        {
            for (size_t i = 0; i < 4; i++)
            {
                _directions[i].second = !(_mmu->readRam(0xFF00) & (1 << i));
                _buttons[i].second = false;
            }
        }
        else
        {
            for (size_t i = 0; i < 4; i++)
            {
                _directions[i].second = false;
                _buttons[i].second = !(_mmu->readRam(0xFF00) & (1 << i));
            }
        }

        // Render registers
        if (ImGui::BeginTable("##Joypad", 2, ImGuiTableFlags_Borders)) // | ImGuiWindowFlags_NoMove))
        {
            ImGui::TableSetupColumn("Buttons", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Directions", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();

            for (size_t i = 0; i < 4; i++)
            {
                ImGui::Checkbox(_buttons[i].first.c_str(), &(_buttons[i].second));
            }

            ImGui::TableNextColumn();

            for (size_t i = 0; i < 4; i++)
            {
                ImGui::Checkbox(_directions[i].first.c_str(), &(_directions[i].second));
            }
            ImGui::EndTable();
        }

        ImGui::End();
    }

    void Debugger::renderMemoryViewerDebugScreen()
    {
        // Set window position and size
        ImGui::SetNextWindowPos(ImVec2(670, 495), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(670, 275), ImGuiCond_Always);

        // Create the window
        ImGui::Begin("Memory", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // Start the tab bar
        if (ImGui::BeginTabBar("##Memory"))
        {
            // Header
            if (ImGui::BeginTabItem("Header"))
            {
                ImGui::Text("Cartridge Header");
                ImGui::EndTabItem();
            }

            // ROM 0 [0 - 0x4000]
            if (ImGui::BeginTabItem("ROM0"))
            {
                ImGui::Text("ROM [0x0 - 0x4000]");
                auto rom = std::span<uint8_t>(_mmu->getCartridge().getRom().begin(), 0x4000);
                showByteArray(rom);
                ImGui::EndTabItem();
            }

            // ROM 1 [0x4000 - 0x8000]
            if (ImGui::BeginTabItem("ROM1"))
            {
                ImGui::Text("ROM [0x4000 - 0x8000] (multiple banks)");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(75);
                showIntegerCombo(1, _mmu->getCartridge()._romBankCount - 1, _currentSelectedRomBank);
                auto rom = std::span<uint8_t>(_mmu->getCartridge().getRom().begin() + _currentSelectedRomBank * 0x4000, 0x4000);
                showByteArray(rom, 0x4000);
                ImGui::EndTabItem();
            }

            // Ext RAM
            if (ImGui::BeginTabItem("SRAM"))
            {
                auto ramBankCount = _mmu->getCartridge()._ramBankCount;
                if (ramBankCount == 0)
                {
                    ImGui::Text("No SRAM");
                }
                else
                {
                    auto &ram = _mmu->getCartridge().getRam();
                    ImGui::Text("SRAM");
                    ImGui::SetNextItemWidth(75);
                    showIntegerCombo(0, ramBankCount - 1, _currentSelectedRamBank);
                    showByteArray(ram, 0x2000 * _currentSelectedRamBank, true);
                }
                ImGui::EndTabItem();
            }

            // VRAM
            if (ImGui::BeginTabItem("VRAM"))
            {
                ImGui::Text("VRAM");
                auto vram = std::span<uint8_t>(_mmu->getMemory().begin() + 0x8000, 0x2000);
                showByteArray(vram, 0x8000);
                ImGui::EndTabItem();
            }

            // OAM
            if (ImGui::BeginTabItem("OAM"))
            {
                ImGui::Text("OAM");
                auto vram = std::span<uint8_t>(_mmu->getMemory().begin() + 0xFE00, 0xA0);
                showByteArray(vram, 0xFE00);
                ImGui::EndTabItem();
            }

            // End the tab bar
            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    void Debugger::renderPreviewSprite()
    {
        if (!_previewSprite.ready)
            return;

        ImGui::Dummy(ImVec2(10, 10));
        ImGui::Text("Attributes: ");

        renderByte("X", [&]()
                   { return static_cast<uint8_t>(_previewSprite.x + 8); }, [&](const uint8_t &) {});

        renderByte("Y", [&]()
                   { return static_cast<uint8_t>(_previewSprite.y + 16); }, [&](const uint8_t &) {});

        renderByte("Tile", [&]()
                   { return static_cast<uint8_t>(_previewSprite.tile); }, [&](const uint8_t &) {});

        showPalette("Palette", _previewSprite.colourPalette, 25, 25);

        _previewSpriteXFlip = (_previewSprite.options.xFlip & 0x1) == 1;
        _previewSpriteYFlip = (_previewSprite.options.yFlip & 0x1) == 1;
        _previewSpritePriority = (_previewSprite.options.renderPriority & 0x1) == 1;

        ImGui::Checkbox("X-Flip", &_previewSpriteXFlip);
        ImGui::SameLine();
        ImGui::Checkbox("Y-Flip", &_previewSpriteYFlip);
        ImGui::SameLine();
        ImGui::Checkbox("Priority", &_previewSpritePriority);

        float tileSize = 48;
        float pixelSize = tileSize / 8.0f;

        // Begin a new ImGui window for the preview
        ImGui::SetNextWindowPos(_previewPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(tileSize * 4, tileSize * 4));
        ImGui::Begin("Sprite Preview", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        // Render the preview of the sprite
        ImDrawList *previewDrawList = ImGui::GetWindowDrawList();
        for (int py = 0; py < 8; ++py)
        {
            for (int px = 0; px < 8; ++px)
            {
                int tileX = _previewSprite.options.xFlip ? (7 - px) : px;
                int tileY = _previewSprite.options.yFlip ? (7 - py) : py;
                uint8_t pixelValue = _mmu->tiles[_previewSprite.tile].pixels[tileY][tileX];
                ImU32 color = PixelToColor(pixelValue);
                previewDrawList->AddRectFilled(
                    ImVec2(_previewPos.x + px * (pixelSize * 4), _previewPos.y + py * (pixelSize * 4)),
                    ImVec2(_previewPos.x + (px + 1) * (pixelSize * 4), _previewPos.y + (py + 1) * (pixelSize * 4)),
                    color);
            }
        }

        ImGui::End();
    }

    void Debugger::renderDisassemblerScreen()
    {
        // Set window position and size
        ImGui::SetNextWindowPos(ImVec2(0, 365), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(670, 400), ImGuiCond_Always);

        // Create the window
        ImGui::Begin("Disassembler", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        uint16_t targetAddress = _registers->PC; // Assume we want to scroll to the current PC
        static uint16_t previousPC = _registers->PC;
        int targetIndex = -1;     // Will store the index of the target address
        int surroundingLines = 5; // Number of lines before/after to show for lazy loading

        if (previousPC != targetAddress)
        {
            _breakPoints.emplace_back(_currentBreakPoint);
            _currentBreakPoint = -1;
        }

        // Find the index of the target address
        for (int i = 0; i < _disassembler.disassembledRom.size(); i++)
        {
            if (_disassembler.disassembledRom[i].address == targetAddress)
            {
                targetIndex = i;
                break;
            }
        }

        if (targetIndex >= 0 && ImGui::BeginTable("##Disassembler", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            // Set headers for the table
            ImGui::TableSetupColumn("Address");
            ImGui::TableSetupColumn("Opcode (Hex)");
            ImGui::TableSetupColumn("Mnemonic");
            ImGui::TableHeadersRow();

            // Scroll to the position of the targetIndex (current PC)
            if (targetIndex >= 0 && previousPC != targetAddress)
            {
                float rowHeight = ImGui::GetTextLineHeightWithSpacing();
                ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y + targetIndex * rowHeight, 0.5f);
                previousPC = targetAddress;
            }

            // Adjust the clipper to show ± `surroundingLines` around the target index
            int clipStart = std::max(0, targetIndex - surroundingLines);
            int clipEnd = static_cast<int>(_disassembler.disassembledRom.size());

            ImGuiListClipper clipper;
            clipper.Begin(clipEnd);
            clipper.DisplayStart = clipStart;
            clipper.DisplayEnd = clipEnd;

            // Pause if any break point hit
            if (_currentBreakPoint < 0 && std::find(_breakPoints.begin(), _breakPoints.end(), targetAddress) != _breakPoints.end())
            {
                Cpu::state = Cpu::State::PAUSED;
                auto it = std::find(_breakPoints.begin(), _breakPoints.end(), targetAddress);
                _breakPoints.erase(it);
                _currentBreakPoint = targetAddress;
            }

            // Lazy load only the visible items
            while (clipper.Step())
            {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                {
                    auto &opcode = _disassembler.disassembledRom[i].opcode;
                    auto address = _disassembler.disassembledRom[i].address;

                    ImGui::TableNextRow();

                    // Create a full row selectable
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TableNextColumn();

                    std::stringstream rowLabel;
                    rowLabel << "##Row" << i; // Use a unique label for each row

                    bool isRowSelected = (i == targetIndex); // Optional: You can track row selection state if needed
                    if (ImGui::Selectable(rowLabel.str().c_str(), isRowSelected, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, 0)))
                    {
                    }

                    // Check if the row was double-clicked
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        auto it = std::find(_breakPoints.begin(), _breakPoints.end(), address);
                        if (it != _breakPoints.end())
                        {
                            std::cout << "Removed breakpoint: " << std::hex << (int)address << std::endl;
                            _breakPoints.erase(it);
                        }
                        else
                        {
                            std::cout << "Added breakpoint: " << std::hex << (int)address << std::endl;
                            _breakPoints.emplace_back(address);
                        }
                    }

                    // Check if the current address is a breakpoint
                    bool isBreakPoint = std::find(_breakPoints.begin(), _breakPoints.end(), address) != _breakPoints.end();

                    // Highlight row for breakpoints (reddish)
                    if (isBreakPoint)
                    {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImVec4(0.8f, 0.2f, 0.2f, 0.65f))); // Reddish background
                    }
                    else if (i == targetIndex)
                    {
                        // Highlight the row corresponding to the target index (PC)
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImVec4(0.2f, 0.4f, 0.9f, 0.65f))); // Highlight row for PC
                    }

                    // Column 0: Address
                    ImGui::TableSetColumnIndex(0);
                    std::stringstream ssAddress;
                    ssAddress << "0x" << std::hex << (int)address;
                    ImGui::Text("%s", ssAddress.str().c_str());

                    // Column 1: Opcode byte(s) in hexadecimal format
                    ImGui::TableSetColumnIndex(1);
                    std::stringstream ss;
                    ss << std::hex << "0x" << (int)opcode.byte << " ";
                    for (auto &byte : opcode.operands)
                    {
                        ss << std::hex << "0x" << (int)byte << " ";
                    }
                    ss << std::endl;
                    ImGui::Text("%s", ss.str().c_str());

                    // Column 2: Mnemonic
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", opcode.mnemonic.c_str());
                }
            }

            // End the table
            ImGui::EndTable();
        }

        ImGui::End();
    }

    void Debugger::RenderSprite(const Mmu::Sprite &sprite)
    {
        if (!sprite.ready)
            return; // Skip rendering if sprite is not ready

        const Mmu::Tile &tile = _mmu->tiles[sprite.tile]; // Get the tile corresponding to the sprite

        // ImGui::GetWindowDrawList() returns the current drawing list to draw custom elements
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos(); // Get the top-left corner for sprite rendering
        float tileSize = 32.0f;                 // Size of the rendered tile in pixels
        float pixelSize = tileSize / 8.0f;      // Each tile is 8x8 pixels

        // Calculate the bounding box for the sprite
        ImVec2 spritePos(p.x, p.y);
        ImVec2 spriteBottomRight(p.x + tileSize, p.y + tileSize);

        // Check if the mouse is hovering over the sprite
        bool isHovered = ImGui::IsMouseHoveringRect(spritePos, spriteBottomRight);

        // Render the sprite
        for (int y = 0; y < 8; ++y)
        {
            for (int x = 0; x < 8; ++x)
            {
                // Get the pixel value from the tile data, applying X and Y flips
                int tileX = sprite.options.xFlip ? (7 - x) : x;
                int tileY = sprite.options.yFlip ? (7 - y) : y;
                uint8_t pixelValue = tile.pixels[tileY][tileX];

                // Get the color from the palette and render the pixel
                ImU32 color = PixelToColor(pixelValue);
                drawList->AddRectFilled(
                    ImVec2(spritePos.x + x * pixelSize, spritePos.y + y * pixelSize),
                    ImVec2(spritePos.x + (x + 1) * pixelSize, spritePos.y + (y + 1) * pixelSize),
                    color);
            }
        }

        // Show a preview if hovering over the sprite
        if (isHovered)
        {
            _previewSprite = sprite;
        }
    }

    void Debugger::renderPpuViewerDebugScreen()
    {
        // Set window position and size
        ImGui::SetNextWindowPos(ImVec2(350, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(700, 365), ImGuiCond_Always);

        // Create the window
        ImGui::Begin("LCD Controller", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        // Start the tab bar
        if (ImGui::BeginTabBar("##LCD Controller"))
        {
            // Control Register
            if (ImGui::BeginTabItem("Control Register"))
            {
                ImGui::Text("Control Register");

                _lcdEnable = _ppu->LCDC->lcdEnable == 1;
                ImGui::Checkbox("LCD Enable", &_lcdEnable);
                if (_lcdEnable != _ppu->LCDC->lcdEnable)
                {
                    _ppu->LCDC->lcdEnable = _lcdEnable;
                }

                ImGui::Text("WINDOW_TILE_MAP_AREA");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(100);
                static const char *windowDisplaySelectValues[] = {"0x9800", "0x9C00"};
                static int windowDisplaySelectValue = 0;
                if (ImGui::Combo("##WINDOW_TILE_MAP_AREA", &windowDisplaySelectValue, windowDisplaySelectValues, IM_ARRAYSIZE(windowDisplaySelectValues)))
                {
                    printf("Selected: %s\n", windowDisplaySelectValues[windowDisplaySelectValue]);
                    _ppu->LCDC->windowDisplaySelect = (windowDisplaySelectValue == 0) ? 0 : 1;
                }

                _windowEnable = _ppu->LCDC->windowEnable == 1;
                ImGui::Checkbox("Window Enable", &_windowEnable);
                if (_windowEnable != _ppu->LCDC->windowEnable)
                {
                    _ppu->LCDC->windowEnable = _windowEnable;
                }

                ImGui::Text("BG_WINDOW_TILE_DATA_MAP_AREA");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(100);
                static const char *bgDisplaySelectValues[] = {"0x9800", "0x9C00"};
                static int bgDisplaySelectValue = 0;
                if (ImGui::Combo("##BG_WINDOW_TILE_DATA_MAP_AREA", &bgDisplaySelectValue, bgDisplaySelectValues, IM_ARRAYSIZE(bgDisplaySelectValues)))
                {
                    printf("Selected: %s\n", bgDisplaySelectValues[bgDisplaySelectValue]);
                    _ppu->LCDC->bgDisplaySelect = (bgDisplaySelectValue == 0) ? 0 : 1;
                }

                _objSize8x8 = _ppu->LCDC->spriteSize == 1;
                ImGui::Checkbox("OBJ Size 8x8", &_objSize8x8);
                if (_objSize8x8 != _ppu->LCDC->spriteSize)
                {
                    _ppu->LCDC->spriteSize = _objSize8x8;
                }

                _objEnabled = _ppu->LCDC->spriteDisplayEnable == 1;
                ImGui::Checkbox("OBJ Enabled", &_objEnabled);
                if (_objEnabled != _ppu->LCDC->spriteDisplayEnable)
                {
                    _ppu->LCDC->spriteDisplayEnable = _objEnabled;
                }

                _bgWindowEnablePriority = _ppu->LCDC->bgDisplay == 1;
                ImGui::Checkbox("BG & Window Enable Priority", &_bgWindowEnablePriority);
                if (_bgWindowEnablePriority != _ppu->LCDC->bgDisplay)
                {
                    _ppu->LCDC->bgDisplay = _bgWindowEnablePriority;
                }

                ImGui::EndTabItem();
            }

            // Status Register
            if (ImGui::BeginTabItem("Status Register"))
            {
                ImGui::Text("Status Register");

                static bool manual = false;
                ImGui::Checkbox("Manual", &manual);

                if (!manual)
                {
                    _scy = _mmu->readRam(0xFF42);
                    _scx = _mmu->readRam(0xFF43);
                    _wy = _mmu->readRam(0xFF4A);
                    if (_wx > 144)
                        _wx = 144;
                    _wx = _mmu->readRam(0xFF4B);
                    if (_wx > 166)
                        _wx = 166;
                    _ly = _mmu->readRam(0xFF44);
                    _lyc = _mmu->readRam(0xFF45);
                }
                else
                {
                    if (_scy != _mmu->readRam(0xff42))
                    {
                        _mmu->writeRam(0xFF42, _scy);
                        _ppu->_debugRender = true;
                    }
                    if (_scx != _mmu->readRam(0xff43))
                    {
                        _mmu->writeRam(0xFF43, _scx);
                        _ppu->_debugRender = true;
                    }
                    if (_wy != _mmu->readRam(0xff4A))
                    {
                        _mmu->writeRam(0xFF4A, _wy);
                        _ppu->_debugRender = true;
                    }
                    if (_wx != _mmu->readRam(0xff4B))
                    {
                        _mmu->writeRam(0xFF4B, _wx);
                        _ppu->_debugRender = true;
                    }

                    if (_ly != _mmu->readRam(0xff44))
                    {
                        _mmu->writeRam(0xFF44, _ly);
                        _ppu->_debugRender = true;
                    }

                    if (_lyc != _mmu->readRam(0xff45))
                    {
                        _mmu->writeRam(0xFF45, _lyc);
                        _ppu->_debugRender = true;
                    }
                }

                ImGui::SliderInt("SCX", &_scx, 0, 255);
                ImGui::SliderInt("SCY", &_scy, 0, 255);
                ImGui::SliderInt("WX", &_wx, 0, 166);
                ImGui::SliderInt("WY", &_wy, 0, 144);
                ImGui::SliderInt("LY", &_ly, 0, 153);
                ImGui::SliderInt("LYC", &_lyc, 0, 153);

                ImGui::EndTabItem();
            }

            // Palette
            if (ImGui::BeginTabItem("Palette"))
            {
                ImGui::Text("Palette");

                showPalette("BGP", _mmu->palette_BGP);
                showPalette("OBP0", _mmu->palette_OBP0);
                showPalette("OBP1", _mmu->palette_OBP1);

                ImGui::EndTabItem();
            }

            // Tiles
            if (ImGui::BeginTabItem("Tiles"))
            {
                ImGui::Text("Tiles");

                int tilesPerRow = 16;              // Number of tiles per row in the grid
                float tileSize = 32.0f;            // Size of each tile on screen
                float pixelSize = tileSize / 8.0f; // Each pixel is drawn as a small square

                for (int tileIndex = 0; tileIndex < 384; ++tileIndex)
                {
                    // Calculate position in grid
                    if (tileIndex > 0 && tileIndex % tilesPerRow != 0)
                        ImGui::SameLine(); // Move to the next tile in the row

                    // Get the draw list to draw custom shapes
                    ImDrawList *drawList = ImGui::GetWindowDrawList();
                    ImVec2 p = ImGui::GetCursorScreenPos(); // Top-left corner of the tile

                    // Loop over each pixel in the tile
                    for (int y = 0; y < 8; ++y)
                    {
                        for (int x = 0; x < 8; ++x)
                        {
                            uint8_t pixelValue = _mmu->tiles[tileIndex].pixels[y][x];
                            ImU32 color = PixelToColor(pixelValue); // Convert pixel value to color

                            // Draw a small square for each pixel
                            drawList->AddRectFilled(
                                ImVec2(p.x + x * pixelSize, p.y + y * pixelSize),             // Top-left corner
                                ImVec2(p.x + (x + 1) * pixelSize, p.y + (y + 1) * pixelSize), // Bottom-right corner
                                color);
                        }
                    }

                    // Move to the next ImGui item (next tile in the grid)
                    ImGui::Dummy(ImVec2(tileSize, tileSize)); // Allocate space for the tile
                }

                ImGui::EndTabItem();
            }

            // DMA
            if (ImGui::BeginTabItem("DMA"))
            {
                ImGui::Text("DMA");
                ImGui::Dummy(ImVec2(0, 10));
                int j = 0;
                for (int i = 0; i < 40; ++i, j++)
                {
                    const Mmu::Sprite &sprite = _mmu->sprites[i];

                    // Only render if the sprite is ready
                    if (!sprite.ready)
                        continue;

                    // Render sprite with its tile and palette
                    RenderSprite(sprite);

                    // Layout
                    ImGui::Dummy(ImVec2(30, 30));
                    if ((i + 1) % 10 != 0)
                    {
                        ImGui::SameLine();
                    }
                }

                renderPreviewSprite();

                ImGui::EndTabItem();
            }

            // End the tab bar
            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    void Debugger::showByteArray(const std::vector<uint8_t> &data, const uint16_t &offset, size_t bytes_per_row)
    {
        ImGuiListClipper clipper;
        clipper.Begin((int)data.size() / (int)bytes_per_row);

        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                size_t i = row * bytes_per_row;

                // Display the memory address (offset)
                ImGui::Text("%04X: ", static_cast<int>(i + offset));

                // Display the data in hex format
                ImGui::SameLine();
                std::stringstream hex_stream;
                for (size_t j = 0; j < bytes_per_row && i + j < data.size(); j++)
                {
                    hex_stream << std::setw(2) << std::setfill('0') << std::hex << (int)data[i + j] << " ";
                }

                std::string hex = hex_stream.str();
                std::transform(hex.begin(), hex.end(), hex.begin(), [](char &c)
                               { return static_cast<char>(std::toupper(static_cast<int>(c))); });
                ImGui::Text("%s", hex.c_str());

                // Display the data in ASCII format (if printable)
                ImGui::SameLine();
                std::stringstream ascii_stream;
                for (size_t j = 0; j < bytes_per_row && i + j < data.size(); j++)
                {
                    uint8_t byte = data[i + j];
                    ascii_stream << (byte >= 32 && byte <= 126 ? static_cast<char>(byte) : '.');
                }
                ImGui::Text("%s", ascii_stream.str().c_str());
            }
        }

        clipper.End();
    }

    void Debugger::showByteArray(std::span<uint8_t> &data, const uint16_t &offset, size_t bytes_per_row, const bool &writable)
    {
        // This variable holds the index of the byte currently being edited.
        static int editedByteIndex = -1;

        ImGuiListClipper clipper;
        int totalRows = (int)data.size() / (int)bytes_per_row + ((data.size() % bytes_per_row) ? 1 : 0);
        clipper.Begin(totalRows);

        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                size_t i = row * bytes_per_row;
                ImGui::Text("%04X: ", static_cast<int>(i + offset));
                ImGui::SameLine();

                // Display the data in hex format.
                for (size_t j = 0; j < bytes_per_row && i + j < data.size(); j++)
                {
                    size_t byteIndex = i + j;
                    // Create a unique label for each byte cell.
                    char label[32];
                    sprintf(label, "##byte_%zu", byteIndex);

                    // Editable hex cell if this byte is being edited.
                    if (editedByteIndex == (int)byteIndex)
                    {
                        ImGui::SetNextItemWidth(20); // Adjust width as needed

                        // Use a temporary variable for editing.
                        uint8_t temp = data[byteIndex];

                        // Push fixed frame padding to ensure constant height when editing.
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4)); // Fixed padding; adjust as needed

                        if (!writable)
                        {
                            ImGui::Text("%02X", temp);
                        }
                        else
                        {
                            if (ImGui::InputScalar(label, ImGuiDataType_U8, &temp, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal))
                            {
                                data[byteIndex] = temp;
                                _mmu->writeRam(byteIndex + offset, temp);
                            }
                            // When done editing, exit edit mode.
                            if (ImGui::IsItemDeactivatedAfterEdit())
                            {
                                editedByteIndex = -1;
                            }
                        }

                        ImGui::PopStyleVar(); // Restore previous style settings.
                    }
                    else
                    {
                        // Display non-editable text.
                        // If the byte is 0x00, display it in dark grey.
                        if (data[byteIndex] == 0x00)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
                        }
                        ImGui::Text("%02X ", data[byteIndex]);
                        if (data[byteIndex] == 0x00)
                        {
                            ImGui::PopStyleColor();
                        }

                        // If the user double clicks this cell, switch to edit mode.
                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                        {
                            editedByteIndex = static_cast<int>(byteIndex);
                        }
                    }
                    ImGui::SameLine();
                }

                // Build the ASCII representation.
                std::stringstream ascii_stream;
                for (size_t j = 0; j < bytes_per_row && i + j < data.size(); j++)
                {
                    uint8_t byte = data[i + j];
                    ascii_stream << (byte >= 32 && byte <= 126 ? static_cast<char>(byte) : '.');
                }
                std::string asciiStr = ascii_stream.str();

                // Get the maximum available width from the content region.
                float contentWidth = ImGui::GetContentRegionMax().x;
                float asciiTextWidth = ImGui::CalcTextSize(asciiStr.c_str()).x;
                // Calculate the target X position such that the ASCII text aligns to the right.
                float targetPosX = contentWidth - asciiTextWidth;
                ImGui::SetCursorPosX(targetPosX);
                ImGui::Text("%s", asciiStr.c_str());
            }
        }

        clipper.End();
    }

    void Debugger::showIntegerCombo(int a, int b, int &selected_value)
    {
        if (ImGui::BeginCombo("Select Bank", std::to_string(selected_value).c_str())) // Combo label
        {
            for (int i = a; i <= b; ++i)
            {
                bool is_selected = (selected_value == i);
                if (ImGui::Selectable(std::to_string(i).c_str(), is_selected))
                {
                    selected_value = i; // Update the selected value
                }

                // Set the initial focus when opening the combo (scroll to the selected value)
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }

    void Debugger::showPalette(const char *label, Colour palette[4], const uint8_t &w, const uint8_t &h)
    {
        ImGui::Text("%s", label);
        for (int i = 0; i < 4; ++i)
        {
            ImVec4 color = ImVec4(palette[i].r / 255.0f, palette[i].g / 255.0f, palette[i].b / 255.0f, palette[i].a / 255.0f);

            // Display a color button for each palette color
            ImGui::ColorButton((std::string("##color") + std::to_string(i + rand() % 255)).c_str(), color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(w, h));

            // Add some space between the buttons
            if (i < 3)
                ImGui::SameLine();
        }
    }

    ImU32 Debugger::PixelToColor(uint8_t pixelValue)
    {
        const Colour &color = _mmu->palette_colours[pixelValue];

        // Convert to ImGui color format (RGBA)
        return IM_COL32(color.r, color.g, color.b, color.a);
    }
}