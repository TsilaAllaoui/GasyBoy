#pragma once

#include <iostream>
#include <SDL2/SDL.h>
#include "mmu.h"
#include "defs.h"

enum {WHITE,LIGHT,DARK,BLACK};

using namespace std;

class Gpu
{
    private:
		//for accessing the gameboy memory
        Mmu *mmu;

		//graphic mode and clock mode
        int graphicMode, line;
        float modeClock;

		//buffer of pixel to show on screen
        int pixels[160][144][3];

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

		//the VRAM Window
		SDL_Window* VramViewer;

		//the gameboy screen Window
		SDL_Window* screen;

		//the VRAM renderer
		SDL_Renderer* VramRenderer;

		//the gameboy screen renderer
		SDL_Renderer* screenRenderer;


    public:
		//constructor/desctructor
        Gpu();
        Gpu(Mmu *p_mmu);
        ~Gpu();

		//one step of the graphic processor unit
        void step(int value);

		//for requesting interrupt
        void requestInterrupt(int id);

		//showing VRAM content
        void showTileData();
        
		//get/set drawing screen status
		bool drawOnScreen();
        void resetDrawScreenStatus();

		//get/set darwing vram status
		bool drawOnVramViewer();
		void resetDrawVramStatus();

		//getting the value of bit at given position
		bool getBitValAt(uint8_t data, int position);
      
		//getters/setters for I/O register of fraphic processor unit
		uint8_t LY();
		void setLY(uint8_t value);
		uint8_t LYC();
		void setLYC(uint8_t value);
        uint8_t SCY();
		void setSCY(uint8_t value);
        uint8_t SCX();
		void setSCX(uint8_t value);
        uint8_t WX();
		void setWX(uint8_t value);
        uint8_t WY();
		void setWY(uint8_t value);
        uint8_t LCDC();
		void setLCDC(uint8_t value);
		uint8_t LCDSTAT();
        void setLCDSTAT(uint8_t value);
		void setLCDStatus();

		//render windows on screen
		void render();

		//drawing scanlines
        void drawScanlines();

		//draw tiles
        void renderTiles();

		//draw scanlines
		void renderScanline(int line);

		//draw sprites
		void renderSprites();


        //void draw_currentline(SDL_Surface *window);
        //void render_sprites(SDL_Surface *window);
        /*void draw_pixels(SDL_Surface *window);*/
        /*void show_tile_line(uint8_t a, uint8_t b, SDL_Surface * screen, int line);
        void show_tile(int i, SDL_Surface *tile);*/
};

