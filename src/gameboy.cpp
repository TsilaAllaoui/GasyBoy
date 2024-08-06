#include "gameboy.h"
#include <ctime>
#include <cstdlib>
#include <fstream>
#define FPS 60


GameBoy::GameBoy( std::string filename )
{
    srand( time( 0 ) );
    
    if( filename == "" )
    {
        filename = new char[100];
        filename = "./Roms/MBC1/PokemonRed.gb";
    }
    
    //initializing SDL App
    if( SDL_Init( SDL_INIT_EVERYTHING ) )
    {
        std::cout << "Error when initializing SDL App.";
        exit( 1 );
    }
    
    mmu = new Mmu( filename );
    cpu = new Cpu( true , mmu );
    gpu = new Gpu( mmu );
    timer = new Timer( mmu );
    interruptHanlder = new Interrupter( mmu, cpu );
    gamepad = mmu->getGamepad();
    cycleCounter = 0;
    
}

GameBoy::~GameBoy()
{
    cpu->~Cpu();
    gpu->~Gpu();
    mmu->~Mmu();
    timer->~Timer();
    gamepad->~Gamepad();
    interruptHanlder->~Interrupter();
    //destroying window && GL context
    SDL_GL_DeleteContext( glcontext );
    SDL_DestroyWindow( window );
    SDL_Quit();
    
}

void GameBoy::step()
{
    int cycle = cpu->step();
    cycleCounter += cycle;
    timer->updateTimer( cycle );
    gpu->step( cycle );
    gamepad->handleEvent();
    interruptHanlder->handleInterrupts();
}

void GameBoy::boot()
{
    bool exit = false;
    int fps = 0;
    int fpsCounter = 0;
    
    fps = SDL_GetTicks();
    
    while( !exit )
    {
        cycleCounter = 0;
        int firstTime = SDL_GetTicks();
        
		while (cycleCounter <= 69905)
		{
			step();

			gpu->render();
		}
        
        //setting main palette
        if( gamepad->changedPalette )
        {
            gpu->changeMainPalette();
            gamepad->changedPalette = false;
        }
        
        
        
        int elapsedTime = SDL_GetTicks() - firstTime;
        
        
        if( elapsedTime < 1000 / FPS )
            SDL_Delay( 1000 / FPS - elapsedTime );
            
        if( SDL_GetTicks() - fps >= 1000 )
        {
            fps = SDL_GetTicks();
            fpsCounter = 0;
        }
        
        else fpsCounter++;
    }
}

