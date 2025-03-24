#ifndef _PPU_H_
#define _PPU_H_

#include <memory>
#include "interruptManager.h"
#include "registers.h"
#include "mmu.h"

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
        Ppu &operator=(const Ppu &);

        std::shared_ptr<Registers> _registers;
        std::shared_ptr<Mmu> _mmu;
        std::shared_ptr<InterruptManager> _interruptManager;

        void reset();

        uint8_t *SCX;
        uint8_t *SCY;
        uint8_t *WX;
        uint8_t *WY;
        uint8_t *LY;
        uint8_t *LCY;

        enum PpuMode
        {
            HBLANK = 0,
            VBLANK = 1,
            OAM_SEARCH = 2,
            DRAWING = 3
        };

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
        } *LCDC;

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
        } *STAT;

        Colour _framebuffer[160 * 144];

        int _modeClock = 0;

        bool _canRender = false;

        void step(const int &cycle);

        void setMode(PpuMode mode);

        void updateLY();
    };
}

#endif