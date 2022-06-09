#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>
#include "defs.h"
#include "gameboy.h"

int main( int argc, char* argv[] )
{
    string rom = ( argc > 1 ) ? argv[1] : "";
    GameBoy( rom ).boot();
    return 0;
}
