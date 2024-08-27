#include <filesystem>
#include <iostream>
#include <string>
#include "logger.h"
#include "argparse.hpp"
#include "gameboy.h"
#include <fstream>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <fstream>
#endif

// Global GameBoy instance
gasyboy::GameBoy *gameBoyInstance = nullptr;

// Function to initialize GameBoy instance
void initializeGameBoy(const std::string &filePath, bool bootBios, bool debugMode)
{
    gameBoyInstance = new gasyboy::GameBoy(filePath, bootBios, debugMode);
}

// Function to clean up GameBoy instance
void cleanupGameBoy()
{
    delete gameBoyInstance;
    gameBoyInstance = nullptr;
}

// Wrapper function for Emscripten's main loop
void gameLoop()
{
    if (gameBoyInstance)
    {
        gameBoyInstance->loop();
    }
}

int main(int argc, char *argv[])
{
#ifdef __EMSCRIPTEN__
    // Initialize GameBoy with default parameters for Emscripten build
    initializeGameBoy("/TETRIS.gb", false, false);
    // Set Emscripten's main loop
    emscripten_set_main_loop(gameLoop, 0, true);
#else
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
        std::string romFile = std::filesystem::path(program.get<std::string>("--rom")).make_preferred().string();
        bool skipBios = program.get<bool>("--skip_bios");
        bool debugMode = program.get<bool>("--debug");

        auto logger = gasyboy::utils::Logger::getInstance();
        logger->log(gasyboy::utils::Logger::LogType::FUNCTIONAL,
                    "Rom file: " + romFile +
                        "\n\t - Use BIOS: " +
                        (!skipBios ? "true" : "false") +
                        "\n\t - Debug Mode: " +
                        (debugMode ? "true" : "false"));

        gasyboy::GameBoy(romFile, !skipBios, debugMode).boot();
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
#endif

    // Clean up (this line may not be reached in Emscripten builds)
    cleanupGameBoy();

    return 0;
}
