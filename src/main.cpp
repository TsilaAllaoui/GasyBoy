#include <filesystem>
#include <iostream>
#include <string>
#include "logger.h"
#include "argparse.hpp"
#include "gameboy.h"
#include <fstream>

std::unique_ptr<gasyboy::GameBoy> gb;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

void main_loop()
{
    if (gb)
    {
        gb->loop();
    }
}

#endif

int main(int argc, char *argv[])
{

#ifdef __EMSCRIPTEN__
    gb = std::make_unique<gasyboy::GameBoy>("/TETRIS.gb", false);

    emscripten_set_main_loop(main_loop, 0, true);
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

        gb = std::make_unique<gasyboy::GameBoy>(romFile, !skipBios, debugMode);
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
#endif

    return 0;
}
