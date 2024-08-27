#ifndef _MMU_H_
#define _MMU_H_

#include <iostream>
#include <fstream>
#include <vector>
#include "cartridge.h"
#include "gamepad.h"
#include "defs.h"

namespace gasyboy
{
    struct Colour
    {
        union
        {
            struct
            {
                uint8_t r, g, b, a;
            };
            uint8_t colours[4];
        };
    };

    class Mmu
    {
    private:
        // internal BIOS of the gameboy
        uint8_t _bios[0x100] =
            {
                0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26,
                0xFF, 0x0E, 0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77,
                0x77, 0x3E, 0xFC, 0xE0, 0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95,
                0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B, 0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06,
                0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9, 0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21,
                0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20, 0xF9, 0x2E, 0x0F, 0x18,
                0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04, 0x1E, 0x02,
                0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
                0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64,
                0x20, 0x06, 0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15,
                0x20, 0xD2, 0x05, 0x20, 0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB,
                0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17, 0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9,
                0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C,
                0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6,
                0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC,
                0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3c, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
                0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE,
                0x34, 0x20, 0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE,
                0x3E, 0x01, 0xE0, 0x50};

        // memory region of the gaameboy
        std::vector<uint8_t> _vRam;
        std::vector<uint8_t> _extRam;
        std::vector<uint8_t> _workingRam;

        // boolean to check if executing from BIOS
        bool _executeBios;

        // context of the gamepad
        Gamepad &_gamepad;

        // the actual cartridge
        Cartridge _cartridge;

        // to check if vram is written
        bool _vramWritten;

        // for storing the modified tiles
        std::vector<int> _modifiedTiles;

        // rom file path
        std::string _romFilePath;

    public:
        // debug mode for tests
        struct mem_access
        {
            int type;
            uint16_t addr;
            uint8_t val;
        };
        int *_num_mem_accesses;
        struct mem_access *_mem_accesses;
        bool _debugMode;
        uint8_t *_mem;
        uint8_t _memSize;

        // the current modified tile
        int _currModifiedTile;

        bool _dmaRegionWritten;

        bool _oamDataIn;

        // construcor/destructor
        Mmu(const std::string &romFilePath, Gamepad &gamepad);

        Mmu(const uint8_t *bytes, const size_t &romSize, Gamepad &gamepad);

        // For debugging
        Mmu(uint8_t size,
            uint8_t *mem,
            int *num_mem_accesses,
            void *mem_accesses,
            Gamepad &gamepad);

        ~Mmu() = default;

        // Set file path to be loaded in cartrdige class
        void setRomFile(const std::string &file);

        // reading/writing into memory
        uint8_t readRam(const uint16_t &adrr);
        void writeRam(const uint16_t &adrr, const uint8_t &value);

        // disabling internal bios && use provided ROM instead
        void disableBios();

        // to check if the gameboy is in internal bios mode
        bool isInBios();

        // graphic memory TODO: change functions names
        std::vector<uint8_t> getVram();
        uint8_t getPaletteColor(const uint8_t &index);

        // directly set value in memory region
        void directSet(const uint16_t &adrr, const uint8_t &value);

        // DMA Transfert routine
        void doDmaTransfert(const uint8_t &value);

        // getting immediate 16bits in low endianess
        uint16_t getNext2Bytes(const uint16_t &adress);

        // get if vram is writen
        bool isVramWritten();

        // set vramWritten status
        void setVramWriteStatus(const bool &value);

        // unset dma written
        void unsetDMAWritten();

        // Get cartridge title
        std::string getCartridgeTitle();

        // Get address of i-th element in ram
        void *ramCellptr(const uint16_t &pos);

        // Usefull structs
        struct Sprite
        {
            bool ready;
            int y;
            int x;
            uint8_t tile;
            Colour *colourPalette;
            struct
            {
                union
                {
                    struct
                    {
                        uint8_t gbcPaletteNumber1 : 1;
                        uint8_t gbcPaletteNumber2 : 1;
                        uint8_t gbcPaletteNumber3 : 1;
                        uint8_t gbcVRAMBank : 1;
                        uint8_t paletteNumber : 1;
                        uint8_t xFlip : 1;
                        uint8_t yFlip : 1;
                        uint8_t renderPriority : 1;
                    };
                    uint8_t value;
                };
            } options;
        } sprites[40] = {Sprite()};

        struct Tile
        {
            uint8_t pixels[8][8] = {0};
        } tiles[384];

        const Colour palette_colours[4] = {
            {255, 255, 255, 255},
            {192, 192, 192, 255},
            {96, 96, 96, 255},
            {0, 0, 0, 255},
        };

        Colour palette_BGP[4] = {
            {255, 255, 255, 255},
            {0, 0, 0, 255},
            {0, 0, 0, 255},
            {0, 0, 0, 255},
        };

        Colour palette_OBP0[4] = {
            {0, 0, 0, 255},
            {0, 0, 0, 255},
            {0, 0, 0, 255},
            {0, 0, 0, 255},
        };

        Colour palette_OBP1[4] = {
            {0, 0, 0, 255},
            {0, 0, 0, 255},
            {0, 0, 0, 255},
            {0, 0, 0, 255},
        };

        void updateTile(uint16_t address, uint8_t value);
        void updateSprite(uint16_t address, uint8_t value);
        void updatePalette(Colour *palette, uint8_t value);
    };
}

#endif