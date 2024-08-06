#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include "gameboy.h"
#include "SDL.h"

int main( int argc, char* argv[] )
{
    std::string rom = ( argc > 1 ) ? argv[1] : "../../externals/gb-test-roms/cpu_instrs/cpu_instrs.gb";

    assert(std::ifstream(rom).is_open());

    GameBoy( rom ).boot();
    
    return 0;
}