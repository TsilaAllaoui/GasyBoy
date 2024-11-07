#ifndef _PPU_H_
#define _PPU_H_

#include "registers.h"
#include "mmu.h"
#include "interruptManager.h"

namespace gasyboy
{
    class Ppu
    {
        void renderScanLines();
        void renderScanLineBackground(bool *rowPixels);
        void renderScanLineWindow();
        void renderScanLineSprites(bool *rowPixels);

    public:
        Ppu();

        Registers &_registers;
        Mmu &_mmu;
        InterruptManager &_interruptManager;

        void reset();

        uint8_t *_scrollX;
        uint8_t *_scrollY;
        uint8_t *_scanline;
        uint32_t _tick;

        struct Control
        {
            union
            {
                struct
                {
                    uint8_t bgDisplay : 1;
                    uint8_t spriteDisplayEnable : 1;
                    uint8_t spriteSize : 1; // True means 8x16 tiles
                    uint8_t bgDisplaySelect : 1;
                    uint8_t bgWindowDataSelect : 1;
                    uint8_t windowEnable : 1;
                    uint8_t windowDisplaySelect : 1;
                    uint8_t lcdEnable : 1;
                };
            };
        } *_control;

        struct Stat
        {
            union
            {
                struct
                {
                    uint8_t modeFlag : 2;
                    uint8_t coincidenceFlag : 1;
                    uint8_t hblankInterrupt : 1;
                    uint8_t vblankInterrupt : 1;
                    uint8_t oamInterrupt : 1;
                    uint8_t coincidenceInterrupt : 1;
                };
            };
        } *_lcdStat;

        Colour _framebuffer[160 * 144];
        // WTF: This variable is unused, but removing her breaks some games
        uint8_t _background[32 * 32];

        int _mode = 0;
        int _modeClock = 0;

        bool _canRender = false;

        void step(const int &cycle);

        void compareLyAndLyc();
    };
}

#endif