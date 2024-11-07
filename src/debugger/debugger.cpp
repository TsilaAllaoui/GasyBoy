#include "interruptManagerProvider.h"
#include "utilitiesProvider.h"
#include "gamepadProvider.h"
#include "ImGuiFileDialog.h"
#include "gameBoyProvider.h"
#include "mmuProvider.h"
#include "cpuProvider.h"
#include "debugger.h"
#include "gameboy.h"
#include "gamepad.h"
#include "logger.h"
#include "mmu.h"
#include <chrono>

#include "registersProvider.h"
#include "utilitiesProvider.h"
#include "timerProvider.h"
#include "mmuProvider.h"
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
          _executeBios(provider::UtilitiesProvider::getInstance().executeBios)
    {
        _bytesBuffers = {
            {"A", new char[2]},
            {"F", new char[2]},
            {"B", new char[2]},
            {"C", new char[2]},
            {"D", new char[2]},
            {"E", new char[2]},
            {"H", new char[2]},
            {"L", new char[2]},
            {"DIV", new char[2]},
            {"TIMA", new char[2]},
            {"TMA", new char[2]},
            {"TAC", new char[2]},
            {"BGP", new char[2]},
            {"OBP0", new char[2]},
            {"OBP1", new char[2]},
            {"X", new char[2]},
            {"Y", new char[2]},
            {"Tile", new char[2]},
            {"Palette", new char[2]},
        };

        _wordsBuffers = {
            {"SP", new char[4]},
            {"PC", new char[4]},
            {"TIMA_INCREMENT_RATE", new char[4]},
            {"WINDOW_TILE_MAP_AREA", new char[4]},
            {"BG_WINDOW_TILE_DATA_MAP_AREA", new char[4]},
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

        // Create the debugger window next to the main window
        _window = SDL_CreateWindow("Debugger",
                                   debuggerWindowX,
                                   debuggerWindowY,
                                   1340, 765, // Width and height for the debugger window
                                   SDL_WINDOW_SHOWN);
        _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

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
        _disassembler.disassemble();
    }

    Debugger::~Debugger()
    {
        for (auto &buffer : _bytesBuffers)
        {
            if (!buffer.second)
            {
                delete buffer.second;
            }
        }

        for (auto &buffer : _wordsBuffers)
        {
            delete buffer.second;
        }

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
        renderDisassemblerScreen();

        ImGui::Render();
        SDL_RenderClear(_renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _renderer);
        SDL_RenderPresent(_renderer);
    }

    void Debugger::renderByte(const std::string &reg, std::function<uint8_t()> get, std::function<void(uint8_t)> set)
    {
        snprintf(_bytesBuffers[reg], sizeof(_bytesBuffers[reg]) + 2, "0x%02X", get());
        std::string regStr = reg;
        regStr += ": ";
        ImGui::Text(regStr.c_str());
        ImGui::SameLine();
        ImGui::SetNextItemWidth(50.0f);
        std::string label = "##" + reg;
        if (ImGui::InputText(label.c_str(), _bytesBuffers[reg], sizeof(_bytesBuffers[reg]), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            auto newValue = static_cast<uint8_t>(std::stoi(_bytesBuffers[reg], nullptr, 16));
            set(newValue);
        }
    }

    void Debugger::renderWord(const std::string &reg, std::function<uint16_t()> get, std::function<void(const uint16_t &value)> set, const size_t &base)
    {
        snprintf(_wordsBuffers[reg], sizeof(_wordsBuffers[reg]) + 2, base == 16 ? "0x%04X" : "%d", get());
        ImGui::Text((reg + ": ").c_str());
        ImGui::SameLine();
        ImGui::SetNextItemWidth(50.0f);
        if (ImGui::InputText(("##" + reg).c_str(), _wordsBuffers[reg], sizeof(_wordsBuffers[reg]), ImGuiInputTextFlags_EnterReturnsTrue))
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
        if (ImGui::BeginTable("##CPU", 2, ImGuiTableFlags_Borders | ImGuiWindowFlags_NoMove))
        {
            ImGui::TableSetupColumn("Registers", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Flags", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();
            renderByte("A", [&]()
                       { return _registers.AF.getLeftRegister(); }, [&](const uint8_t &val)
                       { _registers.AF.setLeftRegister(val); });
            ImGui::SameLine();
            renderByte("F", [&]()
                       { return _registers.AF.getRightRegister(); }, [&](const uint8_t &val)
                       { _registers.AF.setRightRegister(val); });

            renderByte("B", [&]()
                       { return _registers.BC.getLeftRegister(); }, [&](const uint8_t &val)
                       { _registers.BC.setLeftRegister(val); });
            ImGui::SameLine();
            renderByte("C", [&]()
                       { return _registers.BC.getRightRegister(); }, [&](const uint8_t &val)
                       { _registers.BC.setRightRegister(val); });

            renderByte("D", [&]()
                       { return _registers.DE.getLeftRegister(); }, [&](const uint8_t &val)
                       { _registers.DE.setLeftRegister(val); });
            ImGui::SameLine();
            renderByte("E", [&]()
                       { return _registers.DE.getRightRegister(); }, [&](const uint8_t &val)
                       { _registers.DE.setRightRegister(val); });

            renderByte("H", [&]()
                       { return _registers.HL.getLeftRegister(); }, [&](const uint8_t &val)
                       { _registers.HL.setLeftRegister(val); });
            ImGui::SameLine();
            renderByte("L", [&]()
                       { return _registers.HL.getRightRegister(); }, [&](const uint8_t &val)
                       { _registers.HL.setRightRegister(val); });

            renderWord("SP", [&]()
                       { return _registers.SP; }, [&](uint16_t val)
                       { _registers.SP = val; });

            renderWord("PC", [&]()
                       { return _registers.PC; }, [&](uint16_t val)
                       { _registers.PC = val; });

            ImGui::TableNextColumn();

            ImGui::Text("Flags");
            bool Z = _registers.AF.getFlag('Z');
            bool N = _registers.AF.getFlag('N');
            bool H = _registers.AF.getFlag('H');
            bool C = _registers.AF.getFlag('C');
            ImGui::Checkbox("Zero", &Z);
            ImGui::Checkbox("Subtract", &N);
            ImGui::Checkbox("Half Carry", &H);
            ImGui::Checkbox("Carry", &C);

            ImGui::Separator();
            ImGui::Text("State");
            bool halted = _registers.getHalted();
            ImGui::Checkbox("HALTED", &halted);

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
        if (ImGui::Button("Open File Dialog"))
        {
            Cpu::state = Cpu::State::PAUSED;
            IGFD::FileDialogConfig config;
            config.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseRom", "Choose Rom File", ".gb", config);
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseRom", ImGuiWindowFlags_NoCollapse, ImVec2(640, 480)))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                Cpu::state = Cpu::State::PAUSED;
                provider::UtilitiesProvider::getInstance().romFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
                // _registers.reset();
                // _mmu.reset();
                provider::RegistersProvider::getInstance().reset();
                provider::PpuProvider::getInstance().reset();
                provider::MmuProvider::create();
                provider::MmuProvider::getInstance().reset();
                provider::MmuProvider::deleteInstance();
                provider::MmuProvider::create();
                provider::InterruptManagerProvider::getInstance().reset();
                provider::GamepadProvider::getInstance().reset();
            }

            ImGuiFileDialog::Instance()->Close();
            Cpu::state = Cpu::State::RUNNING;
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
        if (ImGui::BeginTable("##Timer", 1, ImGuiTableFlags_Borders | ImGuiWindowFlags_NoMove))
        {
            ImGui::TableSetupColumn("Timer", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();

            renderByte("DIV", [&]()
                       { return _timer.DIV(); }, [&](const uint8_t &value)
                       { _timer.setDIV(value); });

            renderByte("TIMA", [&]()
                       { return _timer.TIMA(); }, [&](const uint8_t &value)
                       { _timer.setTIMA(value); });
            ImGui::SameLine();
            renderWord("TIMA_INCREMENT_RATE", [&]()
                       { return _timer._timaIncrementRate; }, [&](const uint16_t &value)
                       { _timer._timaIncrementRate = value; }, 10);

            renderByte("TMA", [&]()
                       { return _timer.TMA(); }, [&](const uint8_t &value)
                       { _timer.setTMA(value); });

            renderByte("TAC", [&]()
                       { return _timer.TAC(); }, [&](const uint8_t &value)
                       { _timer.setTAC(value); });

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
                _directions[i].second = !(_mmu.readRam(0xFF00) & (1 << i));
                _buttons[i].second = false;
            }
        }
        else
        {
            for (size_t i = 0; i < 4; i++)
            {
                _directions[i].second = false;
                _buttons[i].second = !(_mmu.readRam(0xFF00) & (1 << i));
            }
        }

        // Render registers
        if (ImGui::BeginTable("##Joypad", 2, ImGuiTableFlags_Borders | ImGuiWindowFlags_NoMove))
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
        ImGui::SetNextWindowSize(ImVec2(670, 400), ImGuiCond_Always);

        // Create the window
        ImGui::Begin("Memory", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        auto cartridge = _mmu.getCartridge();

        auto romBanks = _mmu.getCartridge().getRomBanks();

        auto ramBanks = _mmu.getCartridge().getRamBanks();

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
                showByteArray(romBanks.at(0));
                ImGui::EndTabItem();
            }

            // ROM 1 [0x4000 - 0x8000]
            if (ImGui::BeginTabItem("ROM1"))
            {
                ImGui::Text("ROM [0x4000 - 0x8000] (multiple banks)");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(75);
                showIntegerCombo(1, _mmu.getCartridge().getRomBanksNumber() - 1, _currentSelectedRomBank);
                showByteArray(romBanks.at(_currentSelectedRomBank), 0x4000);
                ImGui::EndTabItem();
            }

            // Ext RAM
            if (ImGui::BeginTabItem("Ext RAM"))
            {
                if (cartridge.getRamBanksNumber() == 0)
                {
                    ImGui::Text("No Ext RAM");
                }
                else
                {
                    ImGui::Text("Ext RAM");
                    if (cartridge.getRamBanksNumber() > 2)
                    {
                        showIntegerCombo(0, cartridge.getRamBanksNumber() - 1, _currentSelectedRamBank);
                    }
                    showByteArray(ramBanks.at(_currentSelectedRamBank));
                }
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
                   { return static_cast<uint8_t>(_previewSprite.x); }, [&](const uint8_t &) {});

        renderByte("Y", [&]()
                   { return static_cast<uint8_t>(_previewSprite.y); }, [&](const uint8_t &) {});

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
                uint8_t pixelValue = _mmu.tiles[_previewSprite.tile].pixels[tileY][tileX];
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

        uint16_t targetAddress = _registers.PC; // Assume we want to scroll to the current PC
        static uint16_t previousPC = _registers.PC;
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
                    ImGui::Text(ssAddress.str().c_str());

                    // Column 1: Opcode byte(s) in hexadecimal format
                    ImGui::TableSetColumnIndex(1);
                    std::stringstream ss;
                    ss << std::hex << "0x" << (int)opcode.byte << " ";
                    for (auto &byte : opcode.operands)
                    {
                        ss << std::hex << "0x" << (int)byte << " ";
                    }
                    ss << std::endl;
                    ImGui::Text(ss.str().c_str());

                    // Column 2: Mnemonic
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text(opcode.mnemonic.c_str());
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

        const Mmu::Tile &tile = _mmu.tiles[sprite.tile]; // Get the tile corresponding to the sprite

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

                uint8_t *controlByte = reinterpret_cast<uint8_t *>(_ppu._control);

                _lcdEnable = (*controlByte & (1 << 7)) == 1;
                ImGui::Checkbox("LCD Enable", &_lcdEnable);

                renderWord("WINDOW_TILE_MAP_AREA", [&]()
                           { return static_cast<uint16_t>((*controlByte & (1 << 6)) == 0 ? 0x9800 : 0x9C00); }, [&](const uint16_t &value)
                           {
                            if (value == 0x9800)
                            {
                                _windowTimeMapArea = 0;
                            }
                            else if (value == 0x9C00)
                            {
                                _windowTimeMapArea = 1;
                            }
                            else 
                            {
                               utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG, "Invalid Window Time Map Area!");
                            } });

                _windowEnable = (*controlByte & (1 << 5)) == 1;
                ImGui::Checkbox("Window Enable", &_windowEnable);

                renderWord("BG_WINDOW_TILE_DATA_MAP_AREA", [&]()
                           { return static_cast<uint16_t>((*controlByte & (1 << 4)) == 0 ? 0x9800 : 0x9C00); }, [&](const uint16_t &value)
                           {
                            if (value == 0x9800)
                            {
                                _windowTimeMapArea = 0;
                            }
                            else if (value == 0x9C00)
                            {
                                _windowTimeMapArea = 1;
                            }
                            else 
                            {
                               utils::Logger::getInstance()->log(utils::Logger::LogType::DEBUG, "Invalid Window Time Map Area!");
                            } });

                _objSize8x8 = (*controlByte & (1 << 3)) == 1;
                ImGui::Checkbox("OBJ Size 8x8", &_objSize8x8);
                ImGui::SameLine();
                _objSize8x16 = (*controlByte & (1 << 3)) == 0;
                ImGui::Checkbox("OBJ Size 8x16", &_objSize8x16);

                _objEnabled = (*controlByte & (1 << 2)) == 1;
                ImGui::Checkbox("OBJ Enabled", &_objEnabled);

                _bgWindowEnablePriority = (*controlByte & 0x1) == 1;
                ImGui::Checkbox("BG & Window Enable Priority", &_bgWindowEnablePriority);

                ImGui::EndTabItem();
            }

            // Status Register
            if (ImGui::BeginTabItem("Status Register"))
            {
                ImGui::Text("Status Register");

                _scx = _mmu.readRam(0xFF42);
                ImGui::SliderInt("SCX", &_scx, 0, 255);

                _scy = _mmu.readRam(0xFF43);
                ImGui::SliderInt("SCY", &_scy, 0, 255);

                _wx = _mmu.readRam(0xFF4A);
                ImGui::SliderInt("WX", &_wx, 0, 166);

                _wy = _mmu.readRam(0xFF4B);
                ImGui::SliderInt("WY", &_wy, 0, 144);

                _ly = _mmu.readRam(0xFF44);
                ImGui::SliderInt("LY", &_ly, 0, 153);

                _lyc = _mmu.readRam(0xFF45);
                ImGui::SliderInt("LYC", &_lyc, 0, 153);

                ImGui::EndTabItem();
            }

            // Palette
            if (ImGui::BeginTabItem("Palette"))
            {
                ImGui::Text("Palette");

                showPalette("BGP", _mmu.palette_BGP);
                showPalette("OBP0", _mmu.palette_OBP0);
                showPalette("OBP1", _mmu.palette_OBP1);

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
                            uint8_t pixelValue = _mmu.tiles[tileIndex].pixels[y][x];
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
                    const Mmu::Sprite &sprite = _mmu.sprites[i];

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
            ImGui::ColorButton("##color", color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(w, h));

            // Add some space between the buttons
            if (i < 3)
                ImGui::SameLine();
        }
    }

    ImU32 Debugger::PixelToColor(uint8_t pixelValue)
    {
        const Colour &color = _mmu.palette_colours[pixelValue];

        // Convert to ImGui color format (RGBA)
        return IM_COL32(color.r, color.g, color.b, color.a);
    }
}