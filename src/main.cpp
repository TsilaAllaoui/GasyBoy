#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include "gbException.h"
#include "logger.h"
// #include "argparse.hpp"
// #include "gameboy.h"
// #include "SDL.h"

int main(int argc, char *argv[])
{
    auto logger = gasyboy::utils::Logger::getInstance();
    logger->log("TEST");
    std::cout << logger->getLogContent();
    // argparse::ArgumentParser program("gayboy");

    // program.add_argument("-r", "--rom")
    //     .help("Path to the ROM file")
    //     .required()
    //     .action([](const std::string &value)
    //             { return value; });

    // try
    // {
    //     program.parse_args(argc, argv);
    //     std::string romFile = program.get<std::string>("--rom");
    //     auto logger = gasyboy::utils::Logger::getInstance();
    //     logger->log("ROM file: " + romFile);
    //     GameBoy(romFile).boot();
    // }
    // catch (const std::runtime_error &err)
    // {
    //     std::cout << err.what() << "\n";
    //     std::cout << "usage: gasyboy [-r | --rom rom_file_path]\n\t-r | --rom : the path to the rom file to load\n";
    //     return 1;
    // }

    return 0;
}