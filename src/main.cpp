#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include "gbException.h"
#include "gameboy.h"
#include "SDL.h"
#include "argparse.hpp"

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("gayboy");

    program.add_argument("-r", "--rom")
        .help("Path to the ROM file")
        .required()
        .action([](const std::string &value)
                { return value; });

    try
    {
        program.parse_args(argc, argv);
        std::string romFile = program.get<std::string>("--rom");
        std::cout << "ROM file: " << romFile << "\n";
        GameBoy(romFile).boot();
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what() << "\n";
        return 1;
    }

    return 0;
}