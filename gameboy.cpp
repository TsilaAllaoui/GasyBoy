#include "gameboy.h"
#include <ctime>
#include <cstdlib>

GameBoy::GameBoy(string filename)
{
	srand(time(0));
	if (filename == "")
	{
		filename = new char[100];
		// filename = "./Roms/MBC1/SuperMarioLand.gb";
		filename = "./Roms/NoBanks/TETRIS.gb";
	}

	//initializing SDL App
	if (SDL_Init(SDL_INIT_EVERYTHING))
	{
		std::cout << "Error when initializing SDL App.";
		exit(1);
	}

	//setting up opengl as backend
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	//creating windows
	screen = SDL_CreateWindow("GasyBoy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
	tileMap = SDL_CreateWindow("VRAM Viewer", (SCREEN_WIDTH * SCALE * 3.51), (SCREEN_HEIGHT * SCALE + 108), VRAM_WIDTH * SCALE, VRAM_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
	glcontext = SDL_GL_CreateContext(screen);

	//creating renderer
	screenRenderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
	tileMapRenderer = SDL_CreateRenderer(tileMap, -1, SDL_RENDERER_ACCELERATED);

	mmu = new Mmu(filename);
	cpu = new Cpu(false, mmu);
	//gpu = new Gpu(mmu);
	timer = new Timer(mmu);
	interruptHanlder = new Interrupter(mmu, cpu);
	gamepad = mmu->getGamepad();
	cycleCounter = 0;

}

GameBoy::~GameBoy()
{
	cpu->~Cpu();
	//gpu->~Gpu();
	mmu->~Mmu();
	timer->~Timer();
	gamepad->~Gamepad();
	interruptHanlder->~Interrupter();
	//destroying window and GL context
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();

}

void GameBoy::step()
{
	int cycle = cpu->step();
	cycleCounter += cycle;
	timer->updateTimer(cycle);
	//gpu->step(cycle, window, tile_map);
	gamepad->handleEvent();
	interruptHanlder->handleInterrupts();
}

void GameBoy::boot()
{
	bool exit = false;
	while (!exit)
	{
		cycleCounter = 0;
		while (cycleCounter <= 69905)
			step();

		/*TEST*/

		uint8_t sprite[128] = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0xF0,0x00,0xF0,0x00,0xFC,0x00,0xFC,0x00,0xFC,0x00,0xFC,0x00,0xF3,0x00,0xF3,0x00,
		0x3C,0x00,0x3C,0x00,0x3C,0x00,0x3C,0x00,0x3C,0x00,0x3C,0x00,0x3C,0x00,0x3C,0x00,
		0xF0,0x00,0xF0,0x00,0xF0,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0xF3,0x00,0xF3,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCF,0x00,0xCF,0x00,
		0x00,0x00,0x00,0x00,0x0F,0x00,0x0F,0x00,0x3F,0x00,0x3F,0x00,0x0F,0x00,0x0F,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0xC0,0x00,0x0F,0x00,0x0F,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0xF0,0x00,
		};

		SDL_Texture* textures[8];
		for (int i = 0; i < 8; i++)
			textures[i] = SDL_CreateTexture(tileMapRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 8, 8);

		SDL_SetRenderDrawColor(tileMapRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(tileMapRenderer);

		Uint32 pixels[8 * 8];
		for (int a = 0; a < 8; a++)
		{
			int k = 0;
			for (int j = (a * 16); j < 15 + (a * 16); j += 2)
			{
				uint8_t A = sprite[j], B = sprite[j + 1];
				for (int i = 7; i >= 0; i--)
				{
					if (A & (1 << i))
					{
						if (B & (1 << i))
							pixels[k] = 0x000000FF;
						else if (!(B & (1 << i)))
							pixels[k] = 0x777777FF;
					}
					else
					{
						if (B & (1 << i))
							pixels[k] = 0xCCCCCCFF;
						else if (!(B & (1 << i)))
							pixels[k] = 0xFFFFFFFF;
					}
					k++;
				}
			}
			SDL_UpdateTexture(textures[a], NULL, pixels, 8 * sizeof(Uint32));
		}
		//Update screen

		SDL_Rect pos;
		for (int i = 0; i < 8; i++)
		{
			pos.x = i * 8 * SCALE; pos.y = 0;
			pos.h = 8 * SCALE; pos.w = 8 * SCALE;
			SDL_RenderCopy(tileMapRenderer, textures[i], NULL, &pos);
		}
		SDL_RenderPresent(tileMapRenderer);

		/******/

		SDL_Delay(200);
		/*if (gpu->get_draw())
		{
			gpu->draw_pixels(window);
			if (mmu->read_ram(0xFF40) & (1<<1))
			gpu->render_sprites(window);
			gpu->unset_draw();
			SDL_BlitSurface(window, NULL, screen, NULL);
			SDL_Flip(screen);
		}*/
	}
}

