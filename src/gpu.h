
#ifndef _GPU_H_
#define _GPU_H_

#include <iostream>
#include <map>
#include "defs.h"
#include "SDL.h"
#include "mmu.h"

#define WHITE 0xFFFFFFFF;
#define LIGHTGRAY 0xCCCCCCFF;
#define LIGHTDARK 0x777777FF;
#define BLACK 0x000000FF;

namespace gasyboy
{
    class Sprite
    {
    public:
        uint8_t X, Y, tileNumber, attribute;
        Sprite()
        {
            X = Y = tileNumber = attribute = 0;
        }
        ~Sprite()
        {
        }
    };

    class Gpu
    {
    private:
        // for accessing the gameboy memory
        Mmu &mmu;

        // graphic mode && clock mode
        int graphicMode, line;
        float modeClock;

        // counting LY/scanlines
        int retraceLY;
        int scanlineCounter;

        // for checking if we have to draw screen/vram viewer
        bool drawScreen;
        bool drawTileMap;
        bool drawBG;
        bool drawOAM;
        bool drawWindow;

        // current graphical mode
        int mode;

        // textures representing VRAM Tiles
        SDL_Texture *tilesAt8000[256], *tilesAt9000[128];
        SDL_Texture *tilesForScreenAt8000[256], *tilesForScreenAt9000[128];
        SDL_Texture *tilesForBGAt8000[256], *tilesForBGAt9000[128];
        SDL_Texture *tilesForOAMAt8000[256];
        SDL_Texture *screenTexture;
        SDL_Texture *VramTexture;
        SDL_Texture *BGTexture;
        SDL_Texture *OAMTexture;

        // placeholder for no sprite
        SDL_Texture *placeholder;

        // the VRAM Window
        SDL_Window *VramViewer;

        // the gameboy screen Window
        SDL_Window *screen;

        // the BG screen Window
        SDL_Window *BGViewer;

        // the OAM screen Window
        SDL_Window *OAMViewer;

        // the VRAM renderer
        SDL_Renderer *VramRenderer;

        // the gameboy screen renderer
        SDL_Renderer *screenRenderer;

        // BG renderer
        SDL_Renderer *BGRenderer;

        // OAM renderer
        SDL_Renderer *OAMRenderer;

        // palettes color
        Uint32 *OBP0;
        Uint32 *OBP1;

        // base Palette
        Uint32 *basePalette;

        // all 40 sprites
        Sprite sprites[40];

    public:
        // constructor/desctructor
        Gpu() = default;
        Gpu(Mmu &mmu);
        ~Gpu() = default;

        // one step of the graphic processor unit
        void step(int value);

        // for requesting interrupt
        void requestInterrupt(int id);

        // showing VRAM content
        void showTileData();

        // get/set drawing screen status
        bool drawOnScreen();
        void resetDrawScreenStatus();

        // get/set darwing vram status
        bool drawOnVramViewer();
        void resetDrawVramStatus();
        void resetDrawBG();

        // getting the value of bit at given position
        bool getBitValAt(uint8_t data, int position);

        // getters/setters for I/O register of fraphic processor unit
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

        // render windows on screen
        void render();

        // drawing scanlines
        void drawScanlines();

        // draw sprites
        void renderSprites();

        // draw the window if enabled
        void renderWindow();

        // draw the BG screen if enabled
        void renderBG();

        // render a tile
        void renderTile(uint16_t adress, SDL_Rect *pos, uint16_t colorAdress, bool priority, bool Xflip, bool Yflip);

        // get palette color
        std::vector<uint32_t> getPalette(uint16_t adress);

        // get pixel color on screen
        Uint32 getPixelColor(int x, int y, int bit);

        // change palette on the go
        void changeMainPalette();

        // render a scanline
        void renderCurrScanline(int line);

        // render OAM Viewer
        void renderOAM();

        // getting sprites texture
        // Uint32 *getSpritePixels(uint16_t adress, uint16_t colorAdress);

        // void draw_currentline(SDL_Surface *window);
        // void render_sprites(SDL_Surface *window);
        /*void draw_pixels(SDL_Surface *window);*/
        /*void show_tile_line(uint8_t a, uint8_t b, SDL_Surface * screen, int line);
        void show_tile(int i, SDL_Surface *tile);*/
    };
}

#endif