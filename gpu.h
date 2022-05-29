#pragma once

#include <iostream>
#include <SDL2/SDL.h>
#include "mmu.h"
#include "defs.h"

enum {WHITE,LIGHT,DARK,BLACK};

using namespace std;

class Mmu;

class Gpu
{
    private:
        Mmu *mmu;
        int graphicMode, line;
        float modeClock;
        SDL_Surface *W, *B, *L, *D, *Tile[384];
        int pixels[160][144][3];
        int retraceLY;
        int scanline_counter;
        SDL_Rect pixels_pos;
        bool draw;
		bool drawTileMap;
    public:
        Gpu();
        Gpu(Mmu *p_mmu);
        ~Gpu();
        void increaseScanline();
        uint8_t currScanline();
        void step(int value, SDL_Surface *window, SDL_Surface *tileMap);
        void requestInterrupt(int id);
        void showTileMaps(SDL_Surface *screen);
        void show_tile_line(uint8_t a, uint8_t b, SDL_Surface * screen, int line);
        void show_tile(int i, SDL_Surface *tile);
        void render_tiles();
        uint8_t get_color(int id, uint16_t palette);
        void draw_pixels(SDL_Surface *window);
        uint8_t get_scy();
        uint8_t get_scx();
        uint8_t get_windowx();
        uint8_t get_windowy();
        uint8_t get_LCDC_status();

        bool get_draw();
        void unset_draw();
		bool get_drawTileMap();
		void unset_drawTileMap();
        int BitGetVal(uint8_t data, int position);
        int GetColour(uint8_t colourNum, uint16_t address);
        void render_sprites(SDL_Surface *window);
        void setLCDStatus();
        void draw_currentline(SDL_Surface *window);
        void draw_scanline(SDL_Surface *window);
};

