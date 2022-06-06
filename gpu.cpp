#include "gpu.h"

Gpu::Gpu()
{

}

Gpu::Gpu(Mmu *p_mmu)
{
    mmu = p_mmu;
	mode = 0;
    modeClock = 0;
    graphicMode = 0;
    line = 0;
	drawScreen = false;
	drawTileMap = false;
	scanlineCounter = 456;
    retraceLY = 456;

	//creating VRAM window and renderer
	VramViewer = SDL_CreateWindow("VRAM Viewer", (SCREEN_WIDTH * SCALE * 3.51), (SCREEN_HEIGHT * SCALE + 108), VRAM_WIDTH * SCALE, VRAM_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
	VramRenderer = SDL_CreateRenderer(VramViewer, -1, SDL_RENDERER_ACCELERATED);

	//creating gameboy screen and renderer
	screen = SDL_CreateWindow("GasyBoy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,SCREEN_WIDTH  * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
	screenRenderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);

	//creating all 384 possibles tiles in VRAM
	for (int i = 0; i < 256; i++)
	{
		tilesAt8000[i] = SDL_CreateTexture(VramRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 8, 8);
		tilesForScreenAt8000[i] = SDL_CreateTexture(VramRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 8, 8);
	}
	for (int i = 0; i < 128; i++)
	{
		tilesAt9000[i] = SDL_CreateTexture(VramRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 8, 8);
		tilesForScreenAt9000[i] = SDL_CreateTexture(VramRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 8, 8);
	}
}

Gpu::~Gpu()
{
	mmu->~Mmu();
}

//one PPU step
void Gpu::step(int cycles)
{
	//show VRAM on any changes in memory
	showTileData();

	//set LCD Status changes

	//managing different modes and rendering
	setLCDStatus();
	if (getBitValAt(LCDC(), 7))
		scanlineCounter -= cycles;
	else return;
	if (scanlineCounter <= 0)
	{
		setLY(LY() + 1);
		scanlineCounter = 456;
		if (LY() == 144)
			requestInterrupt(0);
		else if (LY() > 153)
		{
			setLY(0);
			mmu->write_ram(0xFF44, 0);
		}
		else if (LY() < 144)
			drawScanlines();
	}

	//draw on screen on any changes
	render();
}

//setting LCD Status changes and managing modes
void Gpu::setLCDStatus()
{
	uint8_t LcdStat = LCDSTAT();
	if (!getBitValAt(LCDC(), 7))
	{
		scanlineCounter = 456;
		setLY(0);
		mmu->write_ram(0xFF44, 0);
		LcdStat &= 252;
		LcdStat &= ~(1 << 0);
		setLCDSTAT(LcdStat);
		return;
	}
	uint8_t currentMode = LCDSTAT();
	bool reqInt = false;
	if (LY() == 143)
	{
		mode = 1;
		LcdStat |= (1 << 0);
		LcdStat &= ~(1 << 1);
		reqInt = getBitValAt(LcdStat, 4);
		drawScreen = true;
	}
	else
	{
		int mode2Bounds = (456 - 80);
		int mode3Bounds = (mode2Bounds - 172);
		if (scanlineCounter >= mode2Bounds)
		{
			mode = 2;
			LcdStat |= (1 << 1);
			LcdStat &= ~(1 << 0);
			reqInt = getBitValAt(LcdStat, 5);
		}
		else if (scanlineCounter >= mode3Bounds)
		{
			mode = 3;
			LcdStat |= (1 << 1);
			LcdStat |= (1 << 0);
		}
		else
		{
			mode = 0;
			LcdStat &= ~(1 << 1);
			LcdStat &= ~(1 << 0);
			reqInt = getBitValAt(LcdStat, 3);
		}
	}
	if (reqInt && (currentMode != mode))
		requestInterrupt(1);
	if (LY() == LYC())
	{
		LcdStat |= (1 << 2);
		if (getBitValAt(LcdStat, 6))
			requestInterrupt(1);
	}
	else
		LcdStat &= ~(1 << 2);
	setLCDSTAT(LcdStat);
}

//showing VRAM TileData
void Gpu::showTileData()
{
	if (mmu->currModifiedTile >= 0)
	{
		Uint32 pixels[64] = { 0xFFFFFFFF };
		int currPixel = 0;
		int index = mmu->currModifiedTile;
		uint16_t baseAdress = (index < 256) ? 0x8000 : 0x9000;
		uint8_t modifiedTile = (index & 0xFF);
		for (int j = (baseAdress + (modifiedTile << 4)); j < (baseAdress + (modifiedTile << 4) + 15); j += 2)
		{
			uint8_t A = mmu->read_ram(j), B = mmu->read_ram(j + 1);
			for (int bit = 7; bit >= 0; bit--)
			{
				if (getBitValAt(A, bit))
				{
					if (getBitValAt(B, bit))
						pixels[currPixel] = 0x000000FF;
					else
						pixels[currPixel] = 0x777777FF;
				}
				else
				{
					if (getBitValAt(B, bit))
						pixels[currPixel] = 0xCCCCCCFF;
					else
						pixels[currPixel] = 0xFFFFFFFF;
				}
				currPixel++;
			}
		}
		SDL_Surface *tileSurface = NULL;
		if (baseAdress == 0x8000)
		{
			tileSurface = SDL_CreateRGBSurfaceFrom(pixels, 8, 8, 32, 8 * sizeof(Uint32), 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
			tilesAt8000[index] = SDL_CreateTextureFromSurface(VramRenderer, tileSurface);
			tilesForScreenAt8000[index] = SDL_CreateTextureFromSurface(screenRenderer, tileSurface);
		}
		else if (baseAdress == 0x9000)
		{
			tileSurface = SDL_CreateRGBSurfaceFrom(pixels, 8, 8, 32, 8 * sizeof(Uint32), 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
			tilesAt9000[index - 0xFF - 1] = SDL_CreateTextureFromSurface(VramRenderer, tileSurface);
			tilesForScreenAt9000[index - 0xFF - 1] = SDL_CreateTextureFromSurface(screenRenderer, tileSurface);
		}

		SDL_FreeSurface(tileSurface);

		//updating the tile on the window screen
		int y = (int)(((baseAdress + (modifiedTile << 4)) & 0xF00) >> 8);
		if (baseAdress == 0x9000)
			y = y + 16;
		int x = (int)(((baseAdress + (modifiedTile << 4)) & 0xF0) >> 4);
		SDL_Rect pos;
		pos.x = x * 8 * SCALE;
		pos.y = y * 8 * SCALE;
		pos.w = pos.h = 8 * SCALE;
		if (baseAdress == 0x8000)
			SDL_RenderCopy(VramRenderer, tilesAt8000[index], NULL, &pos);
		else SDL_RenderCopy(VramRenderer, tilesAt9000[index - 0xFF - 1], NULL, &pos);
		drawTileMap = true;
		mmu->currModifiedTile = -1;
    }
}

//render all 144 scanlines if LCD is ON
void Gpu::drawScanlines()
{
	if (getBitValAt(LCDC(), 0))
	{
		//draw tiles
		renderTiles();
	}
	if (getBitValAt(LCDC(), 5))
	{
		//render the window
		renderWindow();
	}
	if (getBitValAt(LCDC(), 1))
	{
		//draw sprites
		renderSprites();
	}
}

//render all BC/Window tiles on screen
void Gpu::renderTiles()
{
	SDL_SetRenderDrawColor(screenRenderer, 255, 0, 0, 255);
	SDL_RenderClear(screenRenderer);
	uint16_t tileMap = getBitValAt(LCDC(), 3) ? 0x9C00 : 0x9800;
	uint16_t tileData = getBitValAt(LCDC(), 4) ? 0x8000 : 0x8800;
	uint16_t currTile = tileMap;
	int overY = 0;
	int oldK = 0;
	int oldI = 0;
	for (int k = SCY() / 8; k < SCY() / 8 + 19; k++)
	{
		int x = 0;
		if (k >= 32)
		{
			oldK = k;
			k = overY;
		}
		if (SCX() / 8 + 21 >= 32)
		{
			int over = (SCX() / 8 + 21) - 32;
			for (int i = currTile + 0x20 * k; i < (currTile + 0x20 * k) + ((SCX() / 8 + 21) - over); i++)
			{
				tileData = getBitValAt(LCDC(), 4) ? 0x8000 : 0x8800;
				SDL_Rect dst;
				dst.x = 8 * SCALE * x - SCX() * SCALE;
				if (oldK >= 32)
					dst.y = 8 * SCALE * oldK - SCY() * SCALE;
				else dst.y = 8 * SCALE * k - SCY() * SCALE;
				dst.w = 8 * SCALE;
				dst.h = 8 * SCALE;
				x++;
				if (tileData == 0x8000)
				{
					uint8_t value = mmu->read_ram(i);
					if (value >= 0 && value <= 255)
						SDL_RenderCopy(screenRenderer, tilesForScreenAt8000[value], NULL, &dst);
					else
					{
						uint8_t tmp = value;
						exit(99);
					}
				}
				else if (tileData == 0x8800)
				{
					int8_t value = (int8_t)mmu->read_ram(i);
					if (value >= -128 && value <= 127)
					{
						if (value >= -128 && value < 0)
						{
							SDL_RenderCopy(screenRenderer, tilesForScreenAt8000[256 + value], NULL, &dst);
						}
						else if (value >= 0 && value <= 127)
						{
							SDL_RenderCopy(screenRenderer, tilesForScreenAt9000[value], NULL, &dst);
						}
					}
					else
					{
						uint8_t tmp = value;
						exit(99);
					}
				}
			}
			for (int i = currTile + 0x20 * k; i < (currTile + 0x20 * k) + over; i++)
			{
				tileData = getBitValAt(LCDC(), 4) ? 0x8000 : 0x8800;
				SDL_Rect dst;
				dst.x = 8 * SCALE * x - SCX() * SCALE;
				if (oldK >= 32)
					dst.y = 8 * SCALE * oldK - SCY() * SCALE;
				else dst.y = 8 * SCALE * k - SCY() * SCALE;
				dst.w = 8 * SCALE;
				dst.h = 8 * SCALE;
				x++;
				if (tileData == 0x8000)
				{
					uint8_t value = mmu->read_ram(i);
					if (value >= 0 && value <= 255)
						SDL_RenderCopy(screenRenderer, tilesForScreenAt8000[value], NULL, &dst);
					else
					{
						uint8_t tmp = value;
						exit(99);
					}
				}
				else if (tileData == 0x8800)
				{
					int8_t value = (int8_t)mmu->read_ram(i);
					if (value >= -128 && value <= 127)
					{
						if (value >= -128 && value < 0)
						{
							SDL_RenderCopy(screenRenderer, tilesForScreenAt8000[256 + value], NULL, &dst);
						}
						else if (value >= 0 && value <= 127)
						{
							SDL_RenderCopy(screenRenderer, tilesForScreenAt9000[value], NULL, &dst);
						}
					}
					else
					{
						uint8_t tmp = value;
						exit(99);
					}
				}
			}
		}
		else if (SCX() / 8 + 21 < 32)
		{
			int overX = 0;
			for (int i = currTile + 0x20 * k; i < (currTile + 0x20 * k) + 0x20; i++)
			{
				tileData = getBitValAt(LCDC(), 4) ? 0x8000 : 0x8800;
				SDL_Rect dst;
				dst.x = 8 * SCALE * x - SCX() * SCALE;
				if (oldK >= 32)
					dst.y = 8 * SCALE * oldK - SCY() * SCALE;
				else dst.y = 8 * SCALE * k - SCY() * SCALE;
				dst.w = 8 * SCALE;
				dst.h = 8 * SCALE;
				x++;
				if (tileData == 0x8000)
				{
					uint8_t value = mmu->read_ram(i);
					if (value >= 0 && value <= 255)
						SDL_RenderCopy(screenRenderer, tilesForScreenAt8000[value], NULL, &dst);
					else
					{
						uint8_t tmp = value;
						exit(99);
					}
				}
				else if (tileData == 0x8800)
				{
					int8_t value = (int8_t)mmu->read_ram(i);
					if (value >= -128 && value <= 127)
					{
						if (value >= -128 && value < 0)
						{
							SDL_RenderCopy(screenRenderer, tilesForScreenAt8000[256 + value], NULL, &dst);
						}
						else if (value >= 0 && value <= 127)
						{
							SDL_RenderCopy(screenRenderer, tilesForScreenAt9000[value], NULL, &dst);
						}
					}
					else
					{
						uint8_t tmp = value;
						exit(99);
					}
				}
			}
		}
		if (oldK >= 32)
		{
			k = oldK;
			overY++;
		}
	}
}

//render window if enabled
void Gpu::renderWindow()
{
	if (WY() > LY() + 144)
		return;
	uint16_t tileMap = getBitValAt(LCDC(), 6) ? 0x9C00 : 0x9800;
	uint16_t tileData = getBitValAt(LCDC(), 4) ? 0x8000 : 0x8800;
	uint16_t currTile = tileMap;
	for (int k = 0; k < SCY() + 19; k++)
	{
		int y = 0;
		int x = 0;
		for (int i = currTile + 0x20 * k; i < (currTile + 0x20 * k) + 0x14; i++)
		{
			tileData = getBitValAt(LCDC(), 4) ? 0x8000 : 0x8800;
			SDL_Rect dst;
			dst.x = 8 * SCALE * x + WX() * SCALE - 7 * SCALE;
			dst.y = 8 * SCALE * k + WY() * SCALE;
			dst.w = 8 * SCALE;
			dst.h = 8 * SCALE;
			x++;
			if (tileData == 0x8000)
			{
				uint8_t value = mmu->read_ram(i);
				if (value >= 0 && value <= 255)
					SDL_RenderCopy(screenRenderer, tilesForScreenAt8000[value], NULL, &dst);
				else
				{
					uint8_t tmp = value;
					exit(99);
				}
			}
			else if (tileData == 0x8800)
			{
				int8_t value = (int8_t)mmu->read_ram(i);
				if (value >= -128 && value <= 127)
				{
					if (value >= -128 && value < 0)
					{
						SDL_RenderCopy(screenRenderer, tilesForScreenAt8000[256 + value], NULL, &dst);
					}
					else if (value >= 0 && value <= 127)
					{
						SDL_RenderCopy(screenRenderer, tilesForScreenAt9000[value], NULL, &dst);
					}
				}
				else
				{
					uint8_t tmp = value;
					exit(99);
				}
			}
		}
	}
}

void Gpu::renderSprites()
{
	//iterating through the 40 possibles sprites in OAM
    for (int i=0; i<40; i+=4)
    {
		//texture to be rendered
		SDL_Texture* tile = SDL_CreateTexture(screenRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 8, 8);

		//getting the sprite's informations
        uint8_t yPos = mmu->read_ram(0xFE00 + i) - 16;
        uint8_t xPos = mmu->read_ram(0xFE01 + i) - 8;
        uint8_t tileNumber = mmu->read_ram(0xFE02 + i) ;
        uint8_t attributes = mmu->read_ram(0xFE03 + i) ;

		//the sprites flasg attributes
		bool priority = getBitValAt(attributes, 7);
        bool verticalFlip = getBitValAt(attributes, 6);
        bool horizontalFlip = getBitValAt(attributes, 5);
		bool colorPalette = getBitValAt(attributes, 4);

		//rendering the sprite
        SDL_Rect pos;
        pos.x = xPos * SCALE;
		pos.y = yPos * SCALE;
		pos.h = 8 * SCALE;
		pos.w = 8 * SCALE;

		//counter for the current pixel
		int currPixel = 0;

		//pixels to be filled
		Uint32 pixelData[64] = { 0 };

		//making tile parts transparent
		for (int i = 0x8000 + (tileNumber << 4); i < 0x8000 + (tileNumber << 4) + 15; i += 2)
		{
			uint8_t A = mmu->read_ram(i);
			uint8_t B = mmu->read_ram(i + 1);

			for (int bit = 7; bit >= 0; bit--)
			{
				if (getBitValAt(A, bit))
				{
					if (getBitValAt(B, bit))
						pixelData[currPixel] = 0x000000FF;
					else
						pixelData[currPixel] = 0x777777FF;
				}
				else
				{
					if (getBitValAt(B, bit))
						pixelData[currPixel] = 0xCCCCCCFF;
					else
						pixelData[currPixel] = 0xFFFFFFFF;
				}
				currPixel++;
			}

		}
		SDL_Surface *s = SDL_CreateRGBSurfaceFrom(pixelData, 8, 8, 32, 8 * sizeof(Uint32), 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

		//Transparency
		SDL_SetColorKey(s, SDL_TRUE, SDL_MapRGB(s->format, 255, 255, 255));

		tile = SDL_CreateTextureFromSurface(screenRenderer, s);

		if (verticalFlip)
		{
			//SDL_RenderCopyEx(screenRenderer, tilesForScreenAt8000[tileNumber], NULL, &pos, 0, NULL, SDL_FLIP_VERTICAL);
			SDL_RenderCopyEx(screenRenderer, tile, NULL, &pos, 0, NULL, SDL_FLIP_VERTICAL);
			continue;
		}
		if (horizontalFlip)
		{
			//SDL_RenderCopyEx(screenRenderer, tilesForScreenAt8000[tileNumber], NULL, &pos, 0, NULL, SDL_FLIP_HORIZONTAL);
			SDL_RenderCopyEx(screenRenderer, tile, NULL, &pos, 0, NULL, SDL_FLIP_HORIZONTAL);
			continue;
		}
		if (!horizontalFlip && !verticalFlip)
		{
			//SDL_RenderCopy(screenRenderer, tilesForScreenAt8000[tileNumber], NULL, &pos);
			SDL_RenderCopy(screenRenderer, tile, NULL, &pos);
			continue;
		}
		SDL_FreeSurface(s);
		SDL_DestroyTexture(tile);
    }

}


//render VRAM Viewer/Screen 
void Gpu::render()
{
	if (drawTileMap)
	{
		SDL_RenderPresent(VramRenderer);
		resetDrawVramStatus();
	}
	if (drawScreen)
	{
		SDL_RenderPresent(screenRenderer);
		resetDrawScreenStatus();
	}
}

SDL_Surface* Gpu::renderTile(uint8_t adress)
{
	//counter for the current pixel
	int currPixel = 0;

	//pixels to be filled
	Uint32 pixelData[64] = { 0 };

	//making tile parts transparent
	for (int i = adress; i < adress + 15; i += 2)
	{
		uint8_t A = mmu->read_ram(i);
		uint8_t B = mmu->read_ram(i + 1);

		for (int bit = 7; bit >= 0; bit--)
		{
			if (getBitValAt(A, bit))
			{
				if (getBitValAt(B, bit))
					pixelData[currPixel] = 0x000000FF;
				else
					pixelData[currPixel] = 0x777777FF;
			}
			else
			{
				if (getBitValAt(B, bit))
					pixelData[currPixel] = 0xCCCCCCFF;
				else
					pixelData[currPixel] = 0xFFFFFFFF;
			}
			currPixel++;
		}

	}
	return SDL_CreateRGBSurfaceFrom(pixelData, 8, 8, 32, 8 * sizeof(Uint32), 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
}


uint8_t Gpu::LY()
{
	return mmu->read_ram(0xFF44);
}

void Gpu::setLY(uint8_t value)
{
	mmu->directSet(0xFF44, value);
}

uint8_t Gpu::LYC()
{
	return mmu->read_ram(0xFF45);
}

void Gpu::setLYC(uint8_t value)
{
	mmu->directSet(0xFF45, value);
}

uint8_t Gpu::SCX()
{
	return mmu->read_ram(0xFF43);
}

uint8_t Gpu::SCY()
{
	return mmu->read_ram(0xFF42);
}

uint8_t Gpu::WX()
{
	return mmu->read_ram(0xFF4B);
}

uint8_t Gpu::WY()
{
	return mmu->read_ram(0xFF4A);
}

uint8_t Gpu::LCDC()
{
	return mmu->read_ram(0xFF40);
}

uint8_t Gpu::LCDSTAT()
{
	return mmu->read_ram(0xFF41);
}

void Gpu::setSCX(uint8_t value)
{
	mmu->write_ram(0xFF43, value);
}

void Gpu::setSCY(uint8_t value)
{
	mmu->write_ram(0xFF42, value);
}

void Gpu::setWX(uint8_t value)
{
	mmu->write_ram(0xFF4B, value);
}

void Gpu::setWY(uint8_t value)
{
	mmu->write_ram(0xFF4A, value);
}

void Gpu::setLCDC(uint8_t value)
{
	mmu->write_ram(0xFF40, value);
}

void Gpu::setLCDSTAT(uint8_t value)
{
	mmu->write_ram(0xFF41, value);
}

bool Gpu::drawOnScreen()
{
	return drawScreen;
}

void Gpu::resetDrawScreenStatus()
{
	drawScreen = false;
}

bool Gpu::drawOnVramViewer()
{
	return drawTileMap;
}

void Gpu::resetDrawVramStatus()
{
	drawTileMap = false;
}

void Gpu::requestInterrupt(int id)
{
	uint8_t req = mmu->read_ram(0xFF0F);
	req |= (1 << id);
	mmu->write_ram(0xFF0F, req);
}

bool Gpu::getBitValAt(uint8_t data, int position)
{
	return (data & (1 << position));
}