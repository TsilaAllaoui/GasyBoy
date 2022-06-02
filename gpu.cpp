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
	screen = SDL_CreateWindow("GasyBoy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
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

void Gpu::setLCDStatus()
{
	uint8_t LcdStat = LCDSTAT();
	if (!getBitValAt(LCDC(), 7))
	{
		scanlineCounter = 456;
		setLY(0);
		LcdStat &= 252;
		LcdStat |= (1 << 0);
		setLCDSTAT(LcdStat);
		return;
	}
	uint8_t currentMode = LCDSTAT();
	bool reqInt = false;
	if (LY() >= 144)
	{
		mode = 1;
		LcdStat |= (1 << 0);
		LcdStat &= ~(1 << 1);
		reqInt = getBitValAt(LcdStat, 4);
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


void Gpu::step(int cycles)
{
	showTileMaps();
	setLCDStatus();
	if (getBitValAt(LCDC(), 7))
		scanlineCounter -= cycles;
	else return;
	if (scanlineCounter <= 0)
	{
		setLY(LY() + 1);
		scanlineCounter = 456;
		if (LY() == 144)
		{
			requestInterrupt(0);
			/*drawScreen = true;*/
			drawScreen = true;
		}
		else if (LY() > 153)
		{
			setLY(0);
		}
		else if (LY() < 144)
		{
			//draw on screen
			renderScanline(LY());
		}
	}
}

void Gpu::renderScanline(int line)
{
	uint16_t tileMap = getBitValAt(LCDC(), 3) ? 0x9C00 : 0x9800;
	uint16_t tileData = getBitValAt(LCDC(), 4) ? 0x8000 : 0x8800;
	uint16_t adress;
	if (line > 0)
		line--;
	int x = 0;
	uint16_t currTile = tileMap + 0x20 * (floor((int)line / 8) + floor((int)SCY() / 8));
	for (int i = currTile  ; i < currTile +  0x14; i++)
	{
		tileData = getBitValAt(LCDC(), 4) ? 0x8000 : 0x8800;
		SDL_Rect dst;
		dst.x = 8 * SCALE * x;// -(int)SCX() * SCALE;
		dst.y = SCALE * line;// -SCY() * SCALE;
		dst.w = 8 * SCALE;
		dst.h = SCALE;
		x++;
		SDL_Rect src;
		src.x = 0;
		src.y = line % 8;
		src.w = 8;
		src.h = 1;
		if (tileData == 0x8000)
		{
			uint8_t value = mmu->read_ram(i);
			if (value >= 0 && value <= 255)
				SDL_RenderCopy(screenRenderer, tilesForScreenAt8000[value], &src, &dst);
			else
			{
				uint8_t tmp = value;
				exit(99);
			}
		}
		else if (tileData == 0x8800)
		{
			int8_t value = (int8_t)mmu->read_ram(i);
			if (value >= -127 && value < 128)
			{
				if (value >= -127 && value < 0)
				{
					SDL_RenderCopy(screenRenderer, tilesForScreenAt8000[256 + value], &src, &dst);
				}
				else if (value >= 0 && value < 128)
				{
					SDL_RenderCopy(screenRenderer, tilesForScreenAt9000[value], &src, &dst);
				}
			}
			else
			{
				uint8_t tmp = value;
				exit(99);
			}
		}
	}
	//SDL_RenderPresent(screenRenderer);
	//drawScreen = true;
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

void Gpu::showTileMaps()
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
		SDL_Surface *tileSurface;
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

void Gpu::drawScanlines()
{
	if (getBitValAt(LCDC(), 0))
	{
		//draw tiles
		renderTiles();
	}
	if (getBitValAt(LCDC(), 1))
	{
		//draw sprites
	}
}


void Gpu::renderTiles()
{
	uint16_t tileMap = getBitValAt(LCDC(), 3) ? 0x9C00 : 0x9800;
	uint16_t tileData = getBitValAt(LCDC(), 4) ? 0x8000 : 0x8800;
	uint16_t currTile = tileMap;
	for (int k = 0; k < 32; k++)
	{
		int y = 0;
		int x = 0;
		for (int i = currTile + 0x20 * k; i < (currTile + 0x20 * k) + 0x20; i++)
		{
			tileData = getBitValAt(LCDC(), 4) ? 0x8000 : 0x8800;
			SDL_Rect dst;
			dst.x = 8 * SCALE * x;
			dst.y = 8 * SCALE * k;
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
				if (value >= -127 && value < 128)
				{
					if (value >= -127 && value < 0)
					{
						SDL_RenderCopy(screenRenderer, tilesForScreenAt8000[256 + value], NULL, &dst);
					}
					else if (value >= 0 && value < 128)
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
	SDL_Rect scroll = { SCX() * SCALE, SCY() * SCALE, 160 * SCALE, 144 * SCALE };
	SDL_SetRenderDrawColor(screenRenderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(screenRenderer, &scroll);
	////SDL_RenderPresent(screenRenderer);
	//SDL_RenderPresent(screenRenderer);
	drawScreen = true;
}
 
/*void Gpu::draw_currentline(SDL_Surface *window)
{
    if (!(mmu->read_ram(0xFF40) & (1<<7)))
		return ;

	mmu->directSet(0xFF44, mmu->read_ram(0xFF44) + 1);

	retraceLY = 456 ;

	uint8_t scanLine = mmu->read_ram(0xFF44);

	if ( scanLine == 0x90)
		requestInterrupt(0);

	if (scanLine > 0x99)
		mmu->directSet(0xFF44,0);

	if (scanLine < 0x90)
		draw_scanline(window);
}*/





//void Gpu::render_sprites(SDL_Surface *window)
//{
//    if (mmu->read_ram(0xFF40) & (1<<1))
//    {
//        for (int i=0; i<40; i+=4)
//        {
//            uint8_t yPos = mmu->read_ram(0xFE00+i)- 16;
//            uint8_t xPos = mmu->read_ram(0xFE01+i)-8;
//            uint8_t tileLocation = mmu->read_ram(0xFE02+i) ;
//            uint8_t attributes = mmu->read_ram(0xFE03+i) ;
//
//            bool yFlip = (mmu->read_ram(attributes) & (1<<6)) ;
//            bool xFlip = (mmu->read_ram(attributes) & (1<<5)) ;
//            show_tile(tileLocation,Tile[tileLocation]);
//            SDL_Rect pos;
//            pos.x = xPos * SCALE;
//            pos.y = yPos * SCALE;
//            SDL_BlitSurface(Tile[tileLocation],NULL,window,&pos);
//        }
//    }
//}