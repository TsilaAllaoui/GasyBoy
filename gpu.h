//TODO: Implement Palette for OBJ

#pragma once

#include <iostream>
#include <SDL2/SDL.h>
#include "mmu.h"
#include "defs.h"

#define WHITE 0xFFFFFFFF;
#define LIGHTGRAY 0xCCCCCCFF;
#define LIGHTDARK 0x777777FF;
#define BLACK 0x000000FF;

using namespace std;

class Gpu {
    private:
        //for accessing the gameboy memory
        Mmu* mmu;
        
        //graphic mode and clock mode
        int graphicMode, line;
        float modeClock;
        
        //counting LY/scanlines
        int retraceLY;
        int scanlineCounter;
        
        //for checking if we have to draw screen/vram viewer
        bool drawScreen;
        bool drawTileMap;
        
        //current graphical mode
        int mode;
        
        //textures representing VRAM Tiles
        SDL_Texture* tilesAt8000[256], *tilesAt9000[128];
        SDL_Texture* tilesForScreenAt8000[256], *tilesForScreenAt9000[128];
        SDL_Texture* screenTexture;
        SDL_Texture* VramTexture;
        
        //the VRAM Window
        SDL_Window* VramViewer;
        
        //the gameboy screen Window
        SDL_Window* screen;
        
        //the VRAM renderer
        SDL_Renderer* VramRenderer;
        
        //the gameboy screen renderer
        SDL_Renderer* screenRenderer;
        
        //palettes color
        Uint32* OBP0;
        Uint32* OBP1;
        
        //base Palette
        Uint32* basePalette;
        
        
    public:
        //constructor/desctructor
        Gpu();
        Gpu( Mmu* p_mmu );
        ~Gpu();
        
        //one step of the graphic processor unit
        void step( int value );
        
        //for requesting interrupt
        void requestInterrupt( int id );
        
        //showing VRAM content
        void showTileData();
        
        //get/set drawing screen status
        bool drawOnScreen();
        void resetDrawScreenStatus();
        
        //get/set darwing vram status
        bool drawOnVramViewer();
        void resetDrawVramStatus();
        
        //getting the value of bit at given position
        bool getBitValAt( uint8_t data, int position );
        
        //getters/setters for I/O register of fraphic processor unit
        uint8_t LY();
        void setLY( uint8_t value );
        uint8_t LYC();
        void setLYC( uint8_t value );
        uint8_t SCY();
        void setSCY( uint8_t value );
        uint8_t SCX();
        void setSCX( uint8_t value );
        uint8_t WX();
        void setWX( uint8_t value );
        uint8_t WY();
        void setWY( uint8_t value );
        uint8_t LCDC();
        void setLCDC( uint8_t value );
        uint8_t LCDSTAT();
        void setLCDSTAT( uint8_t value );
        void setLCDStatus();
        
        //render windows on screen
        void render();
        
        //drawing scanlines
        void drawScanlines();
        
        //draw tiles
        void renderTiles();
        
        //draw sprites
        void renderSprites();
        
        //draw the window if enabled
        void renderWindow();
        
        //render a tile
        void renderTile( uint16_t adress, SDL_Rect* pos, uint16_t colorAdress, bool priority, bool Xflip, bool Yflip );
        
        //get palette color
        Uint32* getPalette( uint16_t adress );
        
        //get pixel color on screen
        Uint32 getPixelColor( int x, int y, int bit );
        
        //change palette on the go
        void changeMainPalette();
        
        
        //void draw_currentline(SDL_Surface *window);
        //void render_sprites(SDL_Surface *window);
        /*void draw_pixels(SDL_Surface *window);*/
        /*void show_tile_line(uint8_t a, uint8_t b, SDL_Surface * screen, int line);
        void show_tile(int i, SDL_Surface *tile);*/
};

