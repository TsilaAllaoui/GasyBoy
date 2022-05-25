#include "gameboy.h"

GameBoy::GameBoy(string filename)
{
	if (filename.empty())
	{
		filename = new char[100];
		filename = "C:/Users/Allaoui/Desktop/GasyBoy/Roms/NoBanks/TETRIS.gb";
		//filename = "C:/Users/Allaoui/Desktop/GasyBoy/Roms/MBC1/SuperMarioLand.gb";
	}
        
    mmu = new Mmu(filename);
    cpu = new Cpu(true, mmu);
    gpu = new Gpu(mmu);
    timer = new Timer(mmu);
    interruptHanlder = new Interrupter(mmu, cpu);
    gamepad = mmu->getGamepad();
    cycle_counter = 0;
    SDL_Init(SDL_INIT_VIDEO);
    tile_map = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, VRAM_WIDTH * PX, VRAM_HEIGHT * PX, 32, 0,0,0,0);
    bg_map = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, BG_MAP_WIDTH * PX, BG_MAP_HEIGTH * PX, 32, 0,0,0,0);
    window = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, WIDTH * PX, HEIGHT * PX, 32, 0,0,0,0);
    screen = SDL_SetVideoMode((WIDTH + BG_MAP_WIDTH * 0) * PX,(HEIGHT + VRAM_HEIGHT * 0) * PX, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);

    tile_map_pos.x = 0;
    tile_map_pos.y = HEIGHT * PX;

}

GameBoy::~GameBoy()
{
    cpu->~Cpu();
    gpu->~Gpu();
    mmu->~Mmu();
    timer->~Timer();
    gamepad->~Gamepad();
    interruptHanlder->~Interrupter();
    SDL_FreeSurface(bg_map);
    SDL_FreeSurface(window);
    SDL_FreeSurface(screen);
    SDL_FreeSurface(tile_map);
}

void GameBoy::step()
{
    int cycle = cpu->step();
    cycle_counter += cycle;
    timer->update_timer(cycle);
    gpu->gpuStep(cycle, window);
    gamepad->handleEvent();
    interruptHanlder->handleInterrupts();   
}

void GameBoy::boot()
{
    bool exit = false;
    while (!exit)
    {
        cycle_counter = 0;
        while (cycle_counter <= 69905)
            step();
        if (gpu->get_draw())
        {
            gpu->draw_pixels(window);
              if (mmu->read_ram(0xFF40) & (1<<1))
		    gpu->render_sprites(window);
            gpu->unset_draw();
            SDL_BlitSurface(window, NULL, screen, NULL);
            SDL_Flip(screen);
        }
    }
}

