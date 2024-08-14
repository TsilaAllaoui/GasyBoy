#include <filesystem>
#include <iostream>
#include <string>
#include "logger.h"
#include "argparse.hpp"
#include "gameboy.h"

int main(int argc, char *argv[])
{
    gasyboy::GameBoy("c:/Users/trasoloallaoui/C++/Tetris.gb", true).boot();

    // argparse::ArgumentParser program("gasyboy");

    // program.add_argument("-r", "--rom")
    //     .help("Path to the ROM file")
    //     .required()
    //     .action([](const std::string &value)
    //             { return value; });

    // program.add_argument("-b", "--usebios")
    //     .help("boot with BIOS (true or false)")
    //     .default_value(true)
    //     .implicit_value(true);

    // try
    // {
    //     program.parse_args(argc, argv);
    //     std::string romFile = std::filesystem::path(program.get<std::string>("--rom")).make_preferred().string();
    //     bool useBios = program.get<bool>("--usebios");

    //     auto logger = gasyboy::utils::Logger::getInstance();
    //     logger->log(gasyboy::utils::Logger::LogType::FUNCTIONAL,
    //                 "Rom file: " + romFile + ", Use BIOS: " + (useBios ? "true" : "false"));

    //     gasyboy::GameBoy(romFile, useBios).boot();
    // }
    // catch (const std::runtime_error &err)
    // {
    //     std::cout << err.what() << "\n";
    //     std::cout << "usage: gasyboy [-r | --rom rom_file_path] [--usebios]\n"
    //               << "\t-r | --rom : the path to the rom file to load\n"
    //               << "\t-b | --usebios : use BIOS (default: true)\n";
    //     return 1;
    // }

    return 0;
}