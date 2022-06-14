#include "gpu.h"

Gpu::Gpu()
{

}

Gpu::Gpu( Mmu* p_mmu )
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
    
	//creating gameboy screen and renderer
	screen = SDL_CreateWindow("GasyBoy", 0, 0, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
	screenRenderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
	//setting blend mode for transparency
	SDL_SetRenderDrawBlendMode(screenRenderer, SDL_BLENDMODE_BLEND);

	//getting the main screen position
	int xScreen = 0;
	int yScreen = 0;
	SDL_GetWindowPosition(screen, &xScreen, &yScreen);

    //creating VRAM window and renderer
    VramViewer = SDL_CreateWindow( "VRAM Viewer", xScreen + (SCREEN_WIDTH * SCALE), yScreen, VRAM_WIDTH * SCALE, VRAM_HEIGHT * SCALE, SDL_WINDOW_SHOWN );
    VramRenderer = SDL_CreateRenderer( VramViewer, -1, SDL_RENDERER_ACCELERATED );
    
	//creating BG window and renderer
	BGViewer = SDL_CreateWindow("BG Viewer", xScreen + ((SCREEN_WIDTH + VRAM_WIDTH) * SCALE), yScreen ,32 * 8 * SCALE, 32 * 8 * SCALE, SDL_WINDOW_SHOWN);
	BGRenderer = SDL_CreateRenderer(BGViewer, -1, SDL_RENDERER_ACCELERATED);
    
    //creating all 384 possibles tiles in VRAM
    for( int i = 0; i < 256; i++ )
    {
        tilesAt8000[i] = nullptr;
		tilesForScreenAt8000[i] = nullptr;
		tilesForBGAt8000[i] = nullptr;
    }
    
    for( int i = 0; i < 128; i++ )
    {
        tilesAt9000[i] = nullptr;
        tilesForScreenAt9000[i] = nullptr;
		tilesForBGAt9000[i] = nullptr;
    }
    
    screenTexture = SDL_CreateTexture( screenRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 160 * SCALE, 144 * SCALE );
	VramTexture = SDL_CreateTexture(VramRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, VRAM_WIDTH * SCALE, VRAM_HEIGHT * SCALE);
	BGTexture = SDL_CreateTexture( BGRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 32 * 8 * SCALE, 32 * 8 * SCALE );
    
    //setting up palette
    basePalette = new Uint32[4];
    basePalette[0] = 0xFFFFFFFF;
    basePalette[1] = 0xCCCCCCFF;
    basePalette[2] = 0x777777FF;
    basePalette[3] = 0x000000FF;
}

Gpu::~Gpu()
{
    mmu->~Mmu();
}

//one PPU step
void Gpu::step( int cycles )
{
    //show VRAM on any changes in memory
    showTileData();
    
    //set LCD Status changes
    
    //managing different modes and rendering
    setLCDStatus();
    
    if( getBitValAt( LCDC(), 7 ) )
        scanlineCounter -= cycles;
    else return;
    
    if( scanlineCounter <= 0 )
    {
        setLY( LY() + 1 );
        scanlineCounter = 456;
        
        if( LY() == 144 )
        {
            requestInterrupt( 0 );
            drawScanlines();
        }
        
        else if( LY() > 153 )
        {
            setLY( 0 );
            mmu->write_ram( 0xFF44, 0 );
        }
        
        else if( LY() < 144 )
            renderCurrScanline( ( int )LY() );
    }
}

//setting LCD Status changes and managing modes
void Gpu::setLCDStatus()
{
    uint8_t LcdStat = LCDSTAT();
    
    if( !getBitValAt( LCDC(), 7 ) )
    {
        scanlineCounter = 456;
        setLY( 0 );
        mmu->write_ram( 0xFF44, 0 );
        LcdStat &= 252;
        LcdStat &= ~( 1 << 0 );
        setLCDSTAT( LcdStat );
        return;
    }
    
    uint8_t currentMode = LCDSTAT();
    bool reqInt = false;
    
    if( LY() == 143 )
    {
        mode = 1;
        LcdStat |= ( 1 << 0 );
        LcdStat &= ~( 1 << 1 );
        reqInt = getBitValAt( LcdStat, 4 );
        drawScreen = true;
    }
    
    else
    {
        int mode2Bounds = ( 456 - 80 );
        int mode3Bounds = ( mode2Bounds - 172 );
        
        if( scanlineCounter >= mode2Bounds )
        {
            mode = 2;
            LcdStat |= ( 1 << 1 );
            LcdStat &= ~( 1 << 0 );
            reqInt = getBitValAt( LcdStat, 5 );
        }
        
        else if( scanlineCounter >= mode3Bounds )
        {
            mode = 3;
            LcdStat |= ( 1 << 1 );
            LcdStat |= ( 1 << 0 );
        }
        
        else
        {
            mode = 0;
            LcdStat &= ~( 1 << 1 );
            LcdStat &= ~( 1 << 0 );
            reqInt = getBitValAt( LcdStat, 3 );
        }
    }
    
    if( reqInt && ( currentMode != mode ) )
        requestInterrupt( 1 );
        
    if( LY() == LYC() )
    {
        LcdStat |= ( 1 << 2 );
        
        if( getBitValAt( LcdStat, 6 ) )
            requestInterrupt( 1 );
    }
    
    else
        LcdStat &= ~( 1 << 2 );
        
    setLCDSTAT( LcdStat );
}

//showing VRAM TileData
void Gpu::showTileData()
{
    if( mmu->currModifiedTile >= 0 )
    {
        SDL_SetRenderTarget( VramRenderer, VramTexture );
        Uint32 pixels[64] = { 0xFFFFFFFF };
        int currPixel = 0;
        int index = mmu->currModifiedTile;
        uint16_t baseAdress = ( index < 256 ) ? 0x8000 : 0x9000;
        uint8_t modifiedTile = ( index & 0xFF );
        //Uint32* palette = getPalette( 0xFF47 );
        Uint32* palette = getPalette( 0xFF47 );
        
        for( int j = ( baseAdress + ( modifiedTile << 4 ) ); j < ( baseAdress + ( modifiedTile << 4 ) + 15 ); j += 2 )
        {
            uint8_t B = mmu->read_ram( j );
            uint8_t A = mmu->read_ram( j + 1 );
            
            for( int bit = 7; bit >= 0; bit-- )
            {
                uint8_t color = ( getBitValAt( A, bit ) << 1 ) | getBitValAt( B, bit );
                pixels[currPixel] = palette[color];
                currPixel++;
            }
        }
        
        SDL_Surface* tileSurface = NULL;
        
        if( baseAdress == 0x8000 )
        {
            tileSurface = SDL_CreateRGBSurfaceFrom( pixels, 8, 8, 32, 8 * sizeof( Uint32 ), 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF );
            tilesAt8000[index] = SDL_CreateTextureFromSurface( VramRenderer, tileSurface );
			tilesForScreenAt8000[index] = SDL_CreateTextureFromSurface(screenRenderer, tileSurface);
			tilesForBGAt8000[index] = SDL_CreateTextureFromSurface( BGRenderer, tileSurface );
        }
        
        else if( baseAdress == 0x9000 )
        {
            tileSurface = SDL_CreateRGBSurfaceFrom( pixels, 8, 8, 32, 8 * sizeof( Uint32 ), 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF );
            tilesAt9000[index - 0xFF - 1] = SDL_CreateTextureFromSurface( VramRenderer, tileSurface );
            tilesForScreenAt9000[index - 0xFF - 1] = SDL_CreateTextureFromSurface( screenRenderer, tileSurface );
			tilesForBGAt9000[index - 0xFF - 1] = SDL_CreateTextureFromSurface(BGRenderer, tileSurface);
        }
        
        SDL_FreeSurface( tileSurface );
        
        //updating the tile on the window screen
        int y = ( int )( ( ( baseAdress + ( modifiedTile << 4 ) ) & 0xF00 ) >> 8 );
        
        if( baseAdress == 0x9000 )
            y = y + 16;
            
        int x = ( int )( ( ( baseAdress + ( modifiedTile << 4 ) ) & 0xF0 ) >> 4 );
        SDL_Rect pos;
        pos.x = x * 8 * SCALE;
        pos.y = y * 8 * SCALE;
        pos.w = pos.h = 8 * SCALE;
        
        if( baseAdress == 0x8000 )
            SDL_RenderCopy( VramRenderer, tilesAt8000[index], NULL, &pos );
        else SDL_RenderCopy( VramRenderer, tilesAt9000[index - 0xFF - 1], NULL, &pos );
        
		drawTileMap = true;
        mmu->currModifiedTile = -1;
		SDL_SetRenderTarget(VramRenderer, NULL);
    }
}

//render all 144 scanlines if LCD is ON
void Gpu::drawScanlines()
{
    if( getBitValAt( LCDC(), 0 ) )
    {
        //draw tiles
        drawScreen = true;
		renderBG();
    }
    
    if( getBitValAt( LCDC(), 5 ) )
    {
        //render the window
        renderWindow();
    }
    
    if( getBitValAt( LCDC(), 1 ) )
    {
        //draw sprites
        renderSprites();
    }
}

//render all BC/Window tiles on screen
void Gpu::renderTiles()
{
    SDL_SetRenderDrawColor( screenRenderer, 255, 0, 0, 255 );
    SDL_RenderClear( screenRenderer );
    SDL_SetRenderTarget( screenRenderer, screenTexture );
    uint16_t tileMap = getBitValAt( LCDC(), 3 ) ? 0x9C00 : 0x9800;
    uint16_t tileData = getBitValAt( LCDC(), 4 ) ? 0x8000 : 0x8800;
    uint16_t currTile = tileMap;
    int overY = 0;
    int oldK = 0;
    int oldI = 0;
    int tmp = SCY();
    
    for( int k = tmp / 8; k < tmp / 8 + 19; k++ )
    {
    
    
    
        int x = 0;
        
        if( k >= 32 )
        {
            oldK = k;
            k = overY;
        }
        
        if( SCX() / 8 + 21 >= 32 )
        {
            int over = ( SCX() / 8 + 21 ) - 32;
            
            for( int i = currTile + 0x20 * k; i < ( currTile + 0x20 * k ) + ( ( SCX() / 8 + 21 ) - over ); i++ )
            {
                tileData = getBitValAt( LCDC(), 4 ) ? 0x8000 : 0x8800;
                SDL_Rect dst;
                dst.x = 8 * SCALE * x - SCX() * SCALE;
                
                if( oldK >= 32 )
                    dst.y = 8 * SCALE * oldK - SCY() * SCALE;
                else dst.y = 8 * SCALE * k - SCY() * SCALE;
                
                dst.w = 8 * SCALE;
                dst.h = 8 * SCALE;
                x++;
                
                if( tileData == 0x8000 )
                {
                    uint8_t value = mmu->read_ram( i );
                    
                    if( value >= 0 && value <= 255 )
                        SDL_RenderCopy( screenRenderer, tilesForScreenAt8000[value], NULL, &dst );
                        
                    //renderTile(0x8000 + (value << 4), &dst, 0xFF47);
                    else
                    {
                        uint8_t tmp = value;
                        exit( 99 );
                    }
                }
                
                else if( tileData == 0x8800 )
                {
                    int8_t value = ( int8_t )mmu->read_ram( i );
                    
                    if( value >= -128 && value <= 127 )
                    {
                        if( value >= -128 && value < 0 )
                        {
                            //renderTile(0x8000 + ((value + 256) << 4), &dst, 0xFF47);
                            SDL_RenderCopy( screenRenderer, tilesForScreenAt8000[256 + value], NULL, &dst );
                        }
                        
                        else if( value >= 0 && value <= 127 )
                        {
                            //renderTile(0x8000 + ((value + 128) << 4), &dst, 0xFF47);
                            SDL_RenderCopy( screenRenderer, tilesForScreenAt9000[value], NULL, &dst );
                        }
                    }
                    
                    else
                    {
                        uint8_t tmp = value;
                        exit( 99 );
                    }
                }
            }
            
            for( int i = currTile + 0x20 * k; i < ( currTile + 0x20 * k ) + over; i++ )
            {
                tileData = getBitValAt( LCDC(), 4 ) ? 0x8000 : 0x8800;
                SDL_Rect dst;
                dst.x = 8 * SCALE * x - SCX() * SCALE;
                
                if( oldK >= 32 )
                    dst.y = 8 * SCALE * oldK - SCY() * SCALE;
                else dst.y = 8 * SCALE * k - SCY() * SCALE;
                
                dst.w = 8 * SCALE;
                dst.h = 8 * SCALE;
                x++;
                
                if( tileData == 0x8000 )
                {
                    uint8_t value = mmu->read_ram( i );
                    
                    if( value >= 0 && value <= 255 )
                        //renderTile(0x8000 + (value << 4), &dst, 0xFF47);
                        SDL_RenderCopy( screenRenderer, tilesForScreenAt8000[value], NULL, &dst );
                    else
                    {
                        uint8_t tmp = value;
                        exit( 99 );
                    }
                }
                
                else if( tileData == 0x8800 )
                {
                    int8_t value = ( int8_t )mmu->read_ram( i );
                    
                    if( value >= -128 && value <= 127 )
                    {
                        if( value >= -128 && value < 0 )
                        {
                            //renderTile(0x8000 + ((value + 256) << 4), &dst, 0xFF47);
                            SDL_RenderCopy( screenRenderer, tilesForScreenAt8000[256 + value], NULL, &dst );
                        }
                        
                        else if( value >= 0 && value <= 127 )
                        {
                            //renderTile(0x8000 + ((value + 128) << 4), &dst, 0xFF47);
                            SDL_RenderCopy( screenRenderer, tilesForScreenAt9000[value], NULL, &dst );
                        }
                    }
                    
                    else
                    {
                        uint8_t tmp = value;
                        exit( 99 );
                    }
                }
            }
        }
        
        else if( SCX() / 8 + 21 < 32 )
        {
            int overX = 0;
            
            for( int i = currTile + 0x20 * k; i < ( currTile + 0x20 * k ) + 0x20; i++ )
            {
                tileData = getBitValAt( LCDC(), 4 ) ? 0x8000 : 0x8800;
                SDL_Rect dst;
                dst.x = 8 * SCALE * x - SCX() * SCALE;
                
                if( oldK >= 32 )
                    dst.y = 8 * SCALE * oldK - SCY() * SCALE;
                else dst.y = 8 * SCALE * k - SCY() * SCALE;
                
                dst.w = 8 * SCALE;
                dst.h = 8 * SCALE;
                x++;
                
                if( tileData == 0x8000 )
                {
                    uint8_t value = mmu->read_ram( i );
                    
                    if( value >= 0 && value <= 255 )
                        //renderTile(0x8000 + (value << 4), &dst, 0xFF47);
                        SDL_RenderCopy( screenRenderer, tilesForScreenAt8000[value], NULL, &dst );
                    else
                    {
                        uint8_t tmp = value;
                        exit( 99 );
                    }
                }
                
                else if( tileData == 0x8800 )
                {
                    int8_t value = ( int8_t )mmu->read_ram( i );
                    
                    if( value >= -128 && value <= 127 )
                    {
                        if( value >= -128 && value < 0 )
                        {
                            //renderTile(0x8000 + ((value + 256) << 4), &dst, 0xFF47);
                            SDL_RenderCopy( screenRenderer, tilesForScreenAt8000[256 + value], NULL, &dst );
                        }
                        
                        else if( value >= 0 && value <= 127 )
                        {
                            //renderTile(0x8000 + ((value + 128) << 4), &dst, 0xFF47);
                            SDL_RenderCopy( screenRenderer, tilesForScreenAt9000[value], NULL, &dst );
                        }
                    }
                    
                    else
                    {
                        uint8_t tmp = value;
                        exit( 99 );
                    }
                }
            }
        }
        
        if( oldK >= 32 )
        {
            k = oldK;
            overY++;
        }
    }
    
    SDL_SetRenderTarget( screenRenderer, NULL );
}

//render window if enabled
void Gpu::renderWindow()
{
    SDL_SetRenderTarget( screenRenderer, screenTexture );
    
    if( WY() > LY() + 144 )
        return;
        
    uint16_t tileMap = getBitValAt( LCDC(), 6 ) ? 0x9C00 : 0x9800;
    uint16_t tileData = getBitValAt( LCDC(), 4 ) ? 0x8000 : 0x8800;
    uint16_t currTile = tileMap;
    
    for( int k = 0; k < SCY() + 19; k++ )
    {
        int y = 0;
        int x = 0;
        
        for( int i = currTile + 0x20 * k; i < ( currTile + 0x20 * k ) + 0x14; i++ )
        {
            tileData = getBitValAt( LCDC(), 4 ) ? 0x8000 : 0x8800;
            SDL_Rect dst;
            dst.x = 8 * SCALE * x + WX() * SCALE - 7 * SCALE;
            dst.y = 8 * SCALE * k + WY() * SCALE;
            dst.w = 8 * SCALE;
            dst.h = 8 * SCALE;
            x++;
            
            if( tileData == 0x8000 )
            {
                uint8_t value = mmu->read_ram( i );
                
                if( value >= 0 && value <= 255 )
                    SDL_RenderCopy( screenRenderer, tilesForScreenAt8000[value], NULL, &dst );
                else
                {
                    uint8_t tmp = value;
                    exit( 99 );
                }
            }
            
            else if( tileData == 0x8800 )
            {
                int8_t value = ( int8_t )mmu->read_ram( i );
                
                if( value >= -128 && value <= 127 )
                {
                    if( value >= -128 && value < 0 )
                        SDL_RenderCopy( screenRenderer, tilesForScreenAt8000[256 + value], NULL, &dst );
                    else if( value >= 0 && value <= 127 )
                        SDL_RenderCopy( screenRenderer, tilesForScreenAt9000[value], NULL, &dst );
                }
                
                else
                {
                    uint8_t tmp = value;
                    exit( 99 );
                }
            }
        }
    }
    
    SDL_SetRenderTarget( screenRenderer, NULL );
}

void Gpu::renderBG()
{
	SDL_SetRenderTarget(BGRenderer, BGTexture);
	uint16_t tileMap = getBitValAt(LCDC(), 3) ? 0x9C00 : 0x9800;
	uint16_t tileData = getBitValAt(LCDC(), 4) ? 0x8000 : 0x8800;
	uint16_t currTile = tileMap;

	SDL_Rect dst;

	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			dst.x = 8 * SCALE * j;
			dst.y = 8 * SCALE * i;
			dst.w = 8 * SCALE;
			dst.h = 8 * SCALE;

			uint8_t value = mmu->read_ram(currTile + 0x20 * i + j);

			if (tileData == 0x8000)
			{
				if (value >= 0 && value <= 255)
					SDL_RenderCopy(BGRenderer, tilesForBGAt8000[value], NULL, &dst);
			}
			else if (tileData == 0x8800)
			{
				if (value >= -128 && value <= 127)
				{
					if (value >= -128 && value < 0)
						SDL_RenderCopy(BGRenderer, tilesForBGAt8000[256 + value], NULL, &dst);
					else if (value >= 0 && value <= 127)
						SDL_RenderCopy(BGRenderer, tilesForBGAt9000[value], NULL, &dst);
				}
			}
		}
	}
	//draw the rectangle fomring the viewport
	SDL_Rect rect = { SCX(), SCY(), 160 * SCALE, 144 * SCALE };
	SDL_SetRenderDrawColor(BGRenderer, 255, 0, 0, 255);
	SDL_RenderDrawRect(BGRenderer, &rect);
	drawBG = true;
	SDL_SetRenderTarget(BGRenderer, NULL);
}

void Gpu::renderSprites()
{
    SDL_SetRenderTarget( screenRenderer, screenTexture );
    
    //iterating through the 40 possibles sprites in OAM
    for( int i = 0; i < 0xA0; i += 4 )
    {
        //getting the sprite's informations
        uint8_t yPos = mmu->read_ram( 0xFE00 + i ) - 16;
        uint8_t xPos = mmu->read_ram( 0xFE01 + i ) - 8;
        uint8_t tileNumber = mmu->read_ram( 0xFE02 + i ) ;
        uint8_t attributes = mmu->read_ram( 0xFE03 + i ) ;
        
        
        //the sprites flasg attributes
        bool priority = getBitValAt( attributes, 7 );
        bool verticalFlip = getBitValAt( attributes, 6 );
        bool horizontalFlip = getBitValAt( attributes, 5 );
        bool colorPalette = getBitValAt( attributes, 4 );
        
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
        
        //current used palette adress
        uint16_t adress = ( !colorPalette ) ? 0xFF48 : 0xFF49;
        
        //rendering current sprite
        renderTile( 0x8000 + ( tileNumber << 4 ), &pos, adress, priority, verticalFlip, horizontalFlip );
    }
    
    SDL_SetRenderTarget( screenRenderer, NULL );
}


//render VRAM Viewer/Screen
void Gpu::render()
{
    if( drawTileMap )
    {
        SDL_Rect pos = { 0, 0, VRAM_WIDTH * SCALE, VRAM_HEIGHT * SCALE };
        SDL_RenderCopy( VramRenderer, VramTexture, NULL, &pos );
        SDL_RenderPresent( VramRenderer );
        resetDrawVramStatus();
    }
    
    if( drawScreen )
    {
        SDL_Rect rect = { SCX(), SCY(), 160 * SCALE, 144 * SCALE };
		SDL_RenderCopy(screenRenderer, screenTexture, NULL, &rect);
		SDL_RenderPresent(screenRenderer);
        resetDrawScreenStatus();
    }

	if (drawBG)
	{
		SDL_Rect pos = { 0, 0, 32 * 8 * SCALE, 32 * 8 * SCALE };
		SDL_RenderCopy(BGRenderer, BGTexture, NULL, &pos);
		SDL_RenderPresent(BGRenderer);
		resetDrawBG();
	}
}

void Gpu::renderCurrScanline( int line )
{
    if( line > 0 )
        line--;
        
    uint16_t baseTileIndex = getBitValAt( LCDC(), 3 ) ? 0x9C00 : 0x9800;
    uint16_t tileData = getBitValAt( LCDC(), 4 ) ? 0x8000 : 0x8800;
    
    int X = SCX() / 8;
    int Y = SCY() / 8;
    
    SDL_Rect dst, src;
    
    SDL_Texture* texture = nullptr;
    
    SDL_SetRenderDrawColor( screenRenderer, 0, 255, 0, 255 );
    SDL_RenderClear( screenRenderer );
    SDL_SetRenderTarget( screenRenderer, screenTexture );
    
	int tileYoffset = SCY()/8 +  line / 8;
    
    //for( int i = 0; i < 32; i++ )
    {
        for( int j = X; j < X + 20; j++ )
        {
            uint16_t adress = baseTileIndex + tileYoffset * 32 + j;
            uint8_t value = mmu->read_ram( adress );
            
            //where to render the texture
			dst.y = (line )* SCALE;// -SCY() * SCALE; //i * 8 * SCALE;//
            dst.x = ( j - X ) * SCALE * 8; // j * 8 * SCALE;//
            dst.h = 8 * SCALE; // SCALE;
            dst.w = SCALE * 8;
            
            //source to render
            src.y = ( line % 8 );
            src.x = 0;
            src.h = 1;
            src.w = 8;
            
            //choose tile to render
            if( tileData == 0x8000 )
            {
            
                if( value >= 0 && value <= 255 )
                    texture = tilesForScreenAt8000[value];
            }
            
            else if( tileData == 0x8800 )
            {
                int8_t value = ( int8_t )mmu->read_ram( adress );
                
                if( value >= -128 && value <= 127 )
                {
                    if( value >= -128 && value < 0 )
                        texture = tilesForScreenAt8000[value + 256];
                        
                    else if( value >= 0 && value <= 127 )
                        texture = tilesForScreenAt9000[value];
                }
                
            }
            
            SDL_RenderCopy( screenRenderer, texture, NULL, &dst );
        }
    }
    
    SDL_SetRenderTarget( screenRenderer, NULL );
}

void Gpu::renderTile( uint16_t adress, SDL_Rect* pos, uint16_t colorAdress, bool priority, bool Xflip, bool Yflip )
{
    //counter for the current pixel
    int currPixel = 0;
    
    //pixels to be filled
    Uint32 pixelData[64] = { 0 };
    
    Uint32* palette = getPalette( colorAdress );
    
    //making tile parts transparent
    //getting srpites pixels
    for( int i = adress; i < adress + 15; i += 2 )
    {
        uint8_t B = mmu->read_ram( i );
        uint8_t A = mmu->read_ram( i + 1 );
        
        for( int bit = 7; bit >= 0; bit-- )
        {
            uint8_t color = ( getBitValAt( A, bit ) << 1 ) | getBitValAt( B, bit );
            
            //transparency
            if( palette[color] == palette[0] )
                pixelData[currPixel] = 0;
                
            //if (!priority)
            //{
            //  //TODO
            //  if (palette[color] != 0xFFFFFFFF)
            //      pixelData[currPixel] = 0;
            //}
            else pixelData[currPixel] = palette[color];
            
            currPixel++;
        }
        
    }
    
    //drawing sprites pixels directly on the window
    SDL_Rect tmp = {0, 0, SCALE, SCALE};
    
    for( int i = 0; i < 8; i++ )
    {
        for( int j = 0; j < 8; j++ )
        {
            SDL_SetRenderDrawColor( screenRenderer, ( ( pixelData[i * 8 + j] & 0xFF000000 ) >> 24 ), ( ( pixelData[i * 8 + j] & 0x00FF0000 ) >> 16 ), ( ( pixelData[i * 8 + j] & 0x0000FF00 ) >> 8 ), ( pixelData[i * 8 + j] & 0x000000FF ) );
            
            if( Yflip )
                tmp.x = pos->x + ( ( 7 - j ) * SCALE );
            else tmp.x = pos->x + j * SCALE;
            
            if( Xflip )
                tmp.y = pos->y + ( ( 7 - i ) * SCALE );
            else tmp.y = pos->y + i * SCALE;
            
            SDL_RenderFillRect( screenRenderer, &tmp );
        }
    }
}

Uint32* Gpu::getPalette( uint16_t adress )
{
    uint8_t value = mmu->read_ram( adress );
    //Uint32* mainPalette = Gamepad::basedPalette;
    Uint32 palette[4] = { 0 };
    int k = 0;
    
    for( int i = 0; i < 8; i += 2 )
    {
        switch( ( getBitValAt( value, i ) << 1 ) | getBitValAt( value, i + 1 ) )
        {
            case 0: palette[k] = basePalette[0]; break;
            
            case 1: palette[k] = basePalette[1]; break;
            
            case 2: palette[k] = basePalette[2]; break;
            
            case 3: palette[k] = basePalette[3]; break;
        }
        
        k++;
    }
    
    return palette;
}

Uint32 Gpu::getPixelColor( int x, int y, int bit )
{
    int j = 0x8FE0;// 0x8800 + ((y * 20 + x) << 4);
    uint8_t A = mmu->read_ram( j );
    uint8_t B = mmu->read_ram( j + 1 );
    uint8_t color = ( getBitValAt( A, bit ) << 1 ) | getBitValAt( B, bit );
    
    Uint32* palette = getPalette( 0xFF47 );
    
    
    return palette[color];
}

void Gpu::changeMainPalette()
{
    if( basePalette[0] == 0xFFFFFFFF )
    {
        basePalette[0] = 0x9BBC0FFF;
        basePalette[1] = 0x8BAC0FFF;
        basePalette[2] = 0x306230FF;
        basePalette[3] = 0x0F380FFF;
    }
    
    else if( basePalette[0] == 0x9BBC0FFF )
    {
        basePalette[0] = 0xFFFFFFFF;
        basePalette[1] = 0xCCCCCCFF;
        basePalette[2] = 0x777777FF;
        basePalette[3] = 0x000000FF;
    }
}


uint8_t Gpu::LY()
{
    return mmu->read_ram( 0xFF44 );
}

void Gpu::setLY( uint8_t value )
{
    mmu->directSet( 0xFF44, value );
}

uint8_t Gpu::LYC()
{
    return mmu->read_ram( 0xFF45 );
}

void Gpu::setLYC( uint8_t value )
{
    mmu->directSet( 0xFF45, value );
}

uint8_t Gpu::SCX()
{
    return mmu->read_ram( 0xFF43 );
}

uint8_t Gpu::SCY()
{
    return mmu->read_ram( 0xFF42 );
}

uint8_t Gpu::WX()
{
    return mmu->read_ram( 0xFF4B );
}

uint8_t Gpu::WY()
{
    return mmu->read_ram( 0xFF4A );
}

uint8_t Gpu::LCDC()
{
    return mmu->read_ram( 0xFF40 );
}

uint8_t Gpu::LCDSTAT()
{
    return mmu->read_ram( 0xFF41 );
}

void Gpu::setSCX( uint8_t value )
{
    mmu->write_ram( 0xFF43, value );
}

void Gpu::setSCY( uint8_t value )
{
    mmu->write_ram( 0xFF42, value );
}

void Gpu::setWX( uint8_t value )
{
    mmu->write_ram( 0xFF4B, value );
}

void Gpu::setWY( uint8_t value )
{
    mmu->write_ram( 0xFF4A, value );
}

void Gpu::setLCDC( uint8_t value )
{
    mmu->write_ram( 0xFF40, value );
}

void Gpu::setLCDSTAT( uint8_t value )
{
    mmu->write_ram( 0xFF41, value );
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

void Gpu::resetDrawBG()
{
	drawBG = false;
}

void Gpu::requestInterrupt( int id )
{
    uint8_t req = mmu->read_ram( 0xFF0F );
    req |= ( 1 << id );
    mmu->write_ram( 0xFF0F, req );
}

bool Gpu::getBitValAt( uint8_t data, int position )
{
    return ( data & ( 1 << position ) );
}