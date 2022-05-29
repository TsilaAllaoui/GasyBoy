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
	screen = SDL_CreateWindow("GasyBoy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
	screenRenderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);

	//creating all 384 possibles tiles in VRAM
	for (int i = 0; i < 384; i++)
		tiles[i] = SDL_CreateTexture(VramRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 8, 8);
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
}


void Gpu::step(int cycles)
{
	showTileMaps();
	setLCDStatus();
	if (mmu->read_ram(0xFF40) & (1 << 7))
		scanlineCounter -= cycles;
	else
		return;
	if (scanlineCounter <= 0)
	{
		setLY(LY() + 1);
		scanlineCounter = 456;
		if (LY() == 144)
			requestInterrupt(0);
		else if (LY() > 153)
			setLY(0);
		else if (LY() < 144)
		{
			//draw on screen
		}
	}
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
		Uint32 pixels[64] = {0xFFFFFFFF};
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
			SDL_UpdateTexture(tiles[index], NULL, pixels, 8 * sizeof(Uint32));
			//updating the tile on the window screen
			int y = (int)(((baseAdress + (modifiedTile << 4)) & 0xF00) >> 8);
			if (baseAdress == 0x9000)
				y = y + 16;
			int x = (int)(((baseAdress + (modifiedTile << 4)) & 0xF0) >> 4);
			SDL_Rect pos = { x * 8 * SCALE, y * 8 * SCALE, 8 * SCALE, 8 * SCALE };
			SDL_RenderCopy(VramRenderer, tiles[index], NULL, &pos);
			SDL_RenderCopy(screenRenderer, tiles[index], NULL, &pos);
			
		}
		drawTileMap = true;
		mmu->currModifiedTile = -1;
    }
}
void Gpu::drawScanlines()
{
	if (getBitValAt(LCDC(), 0))
	{
		//draw tiles
		//renderTiles();
	}
	if (getBitValAt(LCDC(), 1))
	{
		//draw sprites
	}
	drawScreen = true;
}


void Gpu::renderTiles()
{

 //   uint16_t tileData = 0 ;
 //   uint16_t backgroundMemory = 0 ;
 //   bool unsig = true ;

 //   uint8_t scrollY = mmu->read_ram(0xFF42) ;
 //   uint8_t scrollX = mmu->read_ram(0xFF43) ;
 //   uint8_t windowY = mmu->read_ram(0xFF4A) ;
 //   uint8_t windowX = mmu->read_ram(0xFF4B) - 7;


	////setting on/off window
 //   bool usingWindow = false ;
 //   if (getBitValAt(LCDC(), 5))
 //       if (WY() <= LY())
 //           usingWindow = true ;

 //   else usingWindow = false ;

	////selecting Window & BG Data set
 //   if (getBitValAt(LCDC(), 4))
 //       tileData = 0x8000 ;
 //   else
 //   {
 //       tileData = 0x8800 ;
 //       unsig = false ;
 //   }

	////selecting BG & Window Tile map to be used
 //   if (!usingWindow)
	//	getBitValAt(LCDC(), 3) ? backgroundMemory = 0x9C00 : backgroundMemory = 0x9800;
	//else getBitValAt(LCDC(), 6) ? backgroundMemory = 0x9C00 : backgroundMemory = 0x9800;
 // 


 //   uint8_t yPos = (!usingWindow) ? SCY() + LY() : LY() - WY();

 //   uint16_t tileRow = (((uint8_t)(yPos/8 ))*32) ;

 //   for (int pixel = 0 ; pixel < 160; pixel++)
 //   {
 //       uint8_t xPos = pixel + SCX();
 //       if (usingWindow)
 //           if (pixel >= WX())
 //               xPos = pixel - WX();

 //       uint16_t tileCol = (xPos/8) ;
 //       int16_t tileNum = (unsig) ? (uint8_t)mmu->read_ram(backgroundMemory+tileRow + tileCol) : tileNum = (int8_t)mmu->read_ram(backgroundMemory+tileRow + tileCol);

 //       uint16_t tileLocation = tileData ;

	//	(unsig) ? tileLocation += (tileNum * 16) : tileLocation += ((tileNum+128) *16) ;

 //       uint8_t line = yPos % 8 ;
 //       line *= 2;
 //       uint8_t data1 = mmu->read_ram(tileLocation + line) ;
 //       uint8_t data2 = mmu->read_ram(tileLocation + line + 1) ;

 //       /*int colourBit = xPos % 8 ;
 //       colourBit -= 7 ;
 //       colourBit *= -1 ;

 //       int colourNum = BitGetVal(data2,colourBit) ;
 //       colourNum <<= 1;
 //       colourNum |= BitGetVal(data1,colourBit) ;

 //       int col = GetColour(colourNum, 0xFF47) ;
 //       int red = 0;
 //       int green = 0;
 //       int blue = 0;

 //       switch(col)
 //       {
 //           case WHITE:
 //               red = 255;
 //               green = 255 ;
 //               blue = 255;
 //               break ;
 //           case LIGHT:
 //               red = 0xCC;
 //               green = 0xCC ;
 //               blue = 0xCC;
 //               break ;
 //           case DARK:
 //               red = 0x77;
 //               green = 0x77 ;
 //               blue = 0x77;
 //               break ;
 //           case BLACK:
 //               red = 0;
 //               green = 0 ;
 //               blue = 0;
 //               break ;
 //       }

 //       int finaly = mmu->read_ram(0xFF44) ;

 //       pixels[pixel][finaly][0] = red ;
 //       pixels[pixel][finaly][1] = green ;
 //       pixels[pixel][finaly][2] = blue ;*/
 //   }
}


//uint8_t Gpu::get_color(int id, uint16_t palette)
//{
//    uint8_t data = mmu->read_ram(palette);
//    int hi = 2 * id + 1, lo = 2 * id;
//    int bit1 = (data >> hi)  & 1;
//    int bit0 = (data >> lo)  & 1;
//    return (bit1 << 1) | bit0;
//}

/*void Gpu::draw_pixels(SDL_Surface *window)
{
  for (int i=0; i<144; i++)
    {
        for (int j=0; j<160; j++)
        {
            int red = pixels[j][i][0], blue = pixels[j][i][1], green = pixels[j][i][2];
            pixels_pos.x = j* SCALE;
            pixels_pos.y = (i - 1)* SCALE;
            if (red == 0xFF && green == 0xFF && blue == 0xFF)
                SDL_BlitSurface(W, NULL, window, &pixels_pos);
            else if (red == 0xCC && green == 0xCC && blue == 0xCC)
                SDL_BlitSurface(D, NULL, window, &pixels_pos);
            else if (red == 0x77 && green == 0x77 && blue == 0x77)
                SDL_BlitSurface(L, NULL, window, &pixels_pos);
            else if (red == 0x0 && green == 0x0 && blue == 0x0)
                SDL_BlitSurface(B, NULL, window, &pixels_pos);
        }
    }
}*/



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