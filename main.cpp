#include "gameboy.h"
#include <SDL/SDL.h>

int main(int argc , char* argv[])
{
    GameBoy(string(argv[1])).boot();
	return 0;
}
