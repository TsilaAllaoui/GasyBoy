#include <filesystem>
#include <iostream>
#include <string>
#include <fstream>

#include "argparse.hpp"
#include "gameboy.h"
#include "logger.h"

#include "interruptManagerProvider.h"
#include "registersProvider.h"
#include "utilitiesProvider.h"
#include "gamepadProvider.h"
#include "gameBoyProvider.h"
#include "timerProvider.h"
#include "mmuProvider.h"
#include "cpuProvider.h"
#include "ppuProvider.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>

void main_loop()
{
    gasyboy::provider::GameBoyProvider::getInstance()->loop();
}

extern "C"
{
    EMSCRIPTEN_KEEPALIVE int load_file(const char *filePath)
    {
        emscripten_cancel_main_loop();

        gasyboy::provider::UtilitiesProvider::getInstance()->romFilePath = filePath;
        gasyboy::provider::GamepadProvider::getInstance()->reset();
        gasyboy::provider::MmuProvider::getInstance()->reset();
        gasyboy::provider::RegistersProvider::getInstance()->reset();
        gasyboy::provider::InterruptManagerProvider::getInstance()->reset();
        gasyboy::provider::CpuProvider::getInstance()->reset();
        gasyboy::provider::TimerProvider::getInstance()->reset();
        gasyboy::provider::PpuProvider::getInstance()->reset();

        emscripten_set_main_loop(main_loop, 0, true);
        return 1;
    }

    EMSCRIPTEN_KEEPALIVE int toggle_bios(const bool value)
    {
        gasyboy::provider::UtilitiesProvider::getInstance()->executeBios = value;
        std::cout << (value ? "Boot bios enabled\n" : "Boot bios disabled\n");
        return 1;
    }
}

int main()
{
    emscripten_set_main_loop(main_loop, 0, true);
    return 0;
}

#else

int main(int argc, char **argv)
{
    gasyboy::provider::UtilitiesProvider::getInstance()->executeBios = true;
    // gasyboy::provider::UtilitiesProvider::getInstance()->romFilePath = "C:/Users/Allaoui/Desktop/GasyBoy/game-boy-collection/Tetris (USA) (Rev-A).gb";
    // gasyboy::provider::UtilitiesProvider::getInstance()->romFilePath = "C:/Users/Allaoui/Desktop/GasyBoy/game-boy-collection/POKEMON_SILVER.gb";
    // gasyboy::provider::UtilitiesProvider::getInstance()->romFilePath = "C:/Users/Allaoui/Desktop/GasyBoy/game-boy-collection/Pokemon Gold Version.gb";
    // gasyboy::provider::UtilitiesProvider::getInstance()->romFilePath = "C:/Users/Allaoui/Desktop/GasyBoy/game-boy-collection/Legend of Zelda, The - Link's Awakening (USA) (Rev-B).gb";
    // gasyboy::provider::UtilitiesProvider::getInstance()->romFilePath = "C:/Users/Allaoui/Desktop/GasyBoy/game-boy-collection/Super Mario Land 2 - 6 Golden Coins (USA) (Rev-B).gb";
    // gasyboy::provider::UtilitiesProvider::getInstance()->romFilePath = "C:/Users/Allaoui/Desktop/GasyBoy/game-boy-collection/Super Mario Land 3 - Wario Land (USA).gb";
    // gasyboy::provider::UtilitiesProvider::getInstance()->romFilePath = "C:/Users/Allaoui/Desktop/GasyBoy/game-boy-collection/dmg-acid2.gb";
    gasyboy::provider::UtilitiesProvider::getInstance()->romFilePath = "C:/Users/Allaoui/Desktop/GasyBoy/gb-test-roms-master/window_y_trigger.gb";

    auto &gb = gasyboy::provider::GameBoyProvider::getInstance();

    // Boot default rom
    if (argc == 1)
    {
        gb->setDebugMode(false);
        gb->boot();
    }

    argparse::ArgumentParser program("gasyboy");

    program.add_argument("-r", "--rom")
        .help("Path to the ROM file")
        .required()
        .action([](const std::string &value)
                { return value; });

    program.add_argument("-s", "--skip_bios")
        .help("skip BIOS on boot")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("-d", "--debug")
        .help("boot in debug mode")
        .default_value(false)
        .implicit_value(true);

    try
    {
        program.parse_args(argc, argv);
        gasyboy::provider::UtilitiesProvider::getInstance()->romFilePath = std::filesystem::path(program.get<std::string>("--rom")).make_preferred().string();
        gasyboy::provider::UtilitiesProvider::getInstance()->executeBios = !program.get<bool>("--skip_bios");
        gasyboy::provider::UtilitiesProvider::getInstance()->debugMode = program.get<bool>("--debug");

        auto logger = gasyboy::utils::Logger::getInstance();
        logger->log(gasyboy::utils::Logger::LogType::FUNCTIONAL,
                    "Rom file: " + gasyboy::provider::UtilitiesProvider::getInstance()->romFilePath +
                        "\n\t - Use BIOS: " +
                        (gasyboy::provider::UtilitiesProvider::getInstance()->executeBios ? "true" : "false") +
                        "\n\t - Debug Mode: " +
                        (gasyboy::provider::UtilitiesProvider::getInstance()->debugMode ? "true" : "false"));

        gb->boot();
    }
    catch (const std::runtime_error &err)
    {
        std::cout << err.what() << "\n";
        std::cout << "usage: gasyboy [-r | --rom rom_file_path] [--usebios]\n"
                  << "\t-r | --rom : the path to the rom file to load\n"
                  << "\t-s | --skip_bios : skip BIOS on boot (default: false)\n"
                  << "\t-d | --debug : boot in debug mode (default: false)\n";
        return 1;
    }

    return 0;
}

#endif
