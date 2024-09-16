#include "debugger.h"
#include "gameboy.h"
#include "gamepad.h"
#include <chrono>

namespace gasyboy
{

    Debugger::Debugger(Mmu &mmu, Registers &registers, Timer &timer, SDL_Window *mainWindow)
        : _mmu(mmu),
          _registers(registers),
          _window(nullptr),
          _renderer(nullptr),
          _breakPoints(),
          _timer(timer),
          _currentSelectedRomBank(1)
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
        };

        _wordsBuffers = {
            {"SP", new char[4]},
            {"PC", new char[4]},
            {"TIMA_INCREMENT_RATE", new char[4]},
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

        // Initialize SDL and ImGui in the new thread
        SDL_Init(SDL_INIT_VIDEO);

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
                                   1050, 495, // Width and height for the debugger window
                                   SDL_WINDOW_SHOWN);
        _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();

        ImGui_ImplSDL2_InitForSDLRenderer(_window, _renderer);
        ImGui_ImplSDLRenderer2_Init(_renderer);
    }

    Debugger::~Debugger()
    {
        for (auto &buffer : _bytesBuffers)
        {
            delete buffer.second;
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
            auto newValue = std::stoi(_bytesBuffers[reg], nullptr, 16);
            set(newValue);
        }
    }

    void Debugger::renderWord(const std::string &reg, std::function<uint16_t()> get, std::function<void(uint16_t)> set, const size_t &base)
    {
        snprintf(_wordsBuffers[reg], sizeof(_wordsBuffers[reg]) + 2, base == 16 ? "0x%04X" : "%d", get());
        ImGui::Text((reg + ": ").c_str());
        ImGui::SameLine();
        ImGui::SetNextItemWidth(50.0f);
        if (ImGui::InputText(("##" + reg).c_str(), _wordsBuffers[reg], sizeof(_wordsBuffers[reg]), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            auto newValue = std::stoi(_wordsBuffers[reg], nullptr, 16);
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
        ImGui::SetNextWindowPos(ImVec2(0, 365), ImGuiCond_Always);
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
        ImGui::SetNextWindowPos(ImVec2(350, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(670, 495), ImGuiCond_Always);

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
                               { return std::toupper(c); });
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

    // Show an integer combo
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
}