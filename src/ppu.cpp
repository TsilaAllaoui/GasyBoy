#include "interruptManagerProvider.h"
#include "registersProvider.h"
#include "mmuProvider.h"
#include "ppu.h"

namespace gasyboy
{
    Ppu::Ppu()
        : _mmu(provider::MmuProvider::getInstance()),
          _registers(provider::RegistersProvider::getInstance()),
          _interruptManager(provider::InterruptManagerProvider::getInstance()),
          _modeClock(0)
    {
        LCDC = reinterpret_cast<Control *>(&_mmu->_memory[0xff40]);
        STAT = reinterpret_cast<Stat *>(&_mmu->_memory[0xff41]);
        SCY = &_mmu->_memory[0xff42];
        SCX = &_mmu->_memory[0xff43];
        LY = &_mmu->_memory[0xff44];
        LCY = &_mmu->_memory[0xff45];
        WY = &_mmu->_memory[0xff4A];
        WX = &_mmu->_memory[0xff4B];
    }

    Ppu &Ppu::operator=(const Ppu &other)
    {
        _mmu = other._mmu;
        _registers = other._registers;
        _interruptManager = other._interruptManager;
        _modeClock = other._modeClock;
        LCDC = other.LCDC;
        STAT = other.STAT;
        SCY = other.SCY;
        SCX = other.SCX;
        LY = other.LY;
        LCY = other.LCY;
        WY = other.WY;
        WX = other.WX;
        for (int i = 0; i < 160 * 144; i++)
            _framebuffer[i] = other._framebuffer[i];
        _canRender = other._canRender;
        return *this;
    }

    void Ppu::step(const int &cycle)
    {
        if (!LCDC->lcdEnable)
        {
            *LY = 0;
            setMode(PpuMode::HBLANK);
            _modeClock = 0;
            return;
        }

        _modeClock += cycle;

        switch (STAT->modeFlag)
        {
        case PpuMode::OAM_SEARCH:
            if (_modeClock >= 80)
            {
                _modeClock -= 80;
                setMode(PpuMode::DRAWING);
            }
            break;

        case PpuMode::DRAWING:
            if (_modeClock >= 172)
            {
                renderScanLines();
                _modeClock -= 172;
                setMode(PpuMode::HBLANK);
            }
            break;

        case PpuMode::HBLANK:
            if (_modeClock >= 204)
            {
                _modeClock -= 204;
                (*LY)++;
                if (*LY == 144)
                {
                    setMode(PpuMode::VBLANK);
                    _interruptManager->requestInterrupt(InterruptManager::InterruptType::VBlank);
                }
                else
                {
                    setMode(PpuMode::OAM_SEARCH);
                }
            }
            break;

        case PpuMode::VBLANK:
            if (_modeClock >= 456)
            {
                _modeClock -= 456;
                (*LY)++;
                if (*LY > 153)
                {
                    _canRender = true;
                    *LY = 0;
                    setMode(PpuMode::OAM_SEARCH);
                }
            }
            break;
        }
    }

    void Ppu::setMode(PpuMode mode)
    {
        STAT->modeFlag = static_cast<uint8_t>(mode);

        if (*LY == *LCY)
        {
            STAT->coincidenceFlag = 1;
            if (STAT->coincidenceInterrupt)
            {
                _interruptManager->requestInterrupt(InterruptManager::InterruptType::LCDStat);
            }
        }
        else
        {
            STAT->coincidenceFlag = 0;
        }

        switch (mode)
        {
        case PpuMode::OAM_SEARCH:
            if (STAT->oamInterrupt)
            {
                _interruptManager->requestInterrupt(InterruptManager::InterruptType::LCDStat);
            }
            break;
        case PpuMode::HBLANK:
            if (STAT->hblankInterrupt)
            {
                _interruptManager->requestInterrupt(InterruptManager::InterruptType::LCDStat);
            }
            break;
        case PpuMode::VBLANK:
            if (STAT->vblankInterrupt)
            {
                _interruptManager->requestInterrupt(InterruptManager::InterruptType::LCDStat);
            }
            break;
        default:
            break;
        }
    }

    void Ppu::renderScanLines()
    {
        bool rowPixels[160] = {0};

        // Check if the Background Enable bit (LCDC.0) is set
        if (LCDC->bgDisplay)
        {
            renderScanLineBackground(rowPixels);

            if (LCDC->windowEnable)
            {
                renderScanLineWindow();
            }
        }

        if (LCDC->spriteDisplayEnable)
        {
            renderScanLineSprites(rowPixels);
        }
    }

    void Ppu::renderScanLineBackground(bool *rowPixels)
    {
        uint16_t address = 0x9800;
        if (LCDC->bgDisplaySelect)
            address += 0x400;

        uint16_t startRowAddress = address + ((*SCY + *LY) / 8) * 32;
        uint16_t endRowAddress = startRowAddress + 32;
        address = startRowAddress + ((*SCX) >> 3);

        int x = *SCX & 7;
        int y = (*LY + *SCY) & 7;
        int pixelOffset = *LY * SCREEN_WIDTH;
        int pixel = 0;
        for (uint16_t i = 0; i < 21; i++)
        {
            uint16_t tile_address = address + i;
            if (tile_address >= endRowAddress)
                tile_address = startRowAddress + (tile_address % 32);

            int tile = _mmu->readRam(tile_address);
            if (!LCDC->bgWindowDataSelect && tile < 128)
                tile += 256;

            for (; x < 8 && pixel < 160; x++)
            {
                if (pixelOffset >= SCREEN_WIDTH * SCREEN_HEIGHT)
                    return;
                int colour = _mmu->tiles[tile].pixels[y][x];
                _framebuffer[pixelOffset++] = _mmu->palette_BGP[colour];
                if (colour > 0)
                    rowPixels[pixel] = true;
                pixel++;
            }
            x = 0;
        }
    }

    void Ppu::renderScanLineWindow()
    {
        if (*WY > *LY || *WX > 160 || *WX <= 0)
            return;

        uint16_t address = 0x9800;
        if (LCDC->windowDisplaySelect)
            address += 0x400;
        *WX = -7;
        int y = (*LY - *WY) & 7;
        int pixelOffset = *LY * SCREEN_WIDTH;
        address += ((*LY - *WY) / 8) * 32;

        for (uint16_t tileX = 0; tileX < 21; tileX++)
        {
            uint16_t tile_address = address + tileX;
            int tile = _mmu->readRam(tile_address);
            if (!LCDC->bgWindowDataSelect && tile < 128)
                tile += 256;

            for (int x = 0; x < 8; x++)
            {
                int windowPixelX = *WX + tileX * 8 + x;
                if (windowPixelX < 0 || windowPixelX >= SCREEN_WIDTH)
                    continue;
                int colour = _mmu->tiles[tile].pixels[y][x];
                if (pixelOffset + windowPixelX >= SCREEN_WIDTH * SCREEN_HEIGHT)
                    return;
                _framebuffer[pixelOffset + windowPixelX] = _mmu->palette_BGP[colour];
            }
        }
    }

    void Ppu::renderScanLineSprites(bool *rowPixels)
    {
        int sprite_height = LCDC->spriteSize ? 16 : 8;
        int spritesRendered = 0;

        for (int i = 0; i < 40; i++)
        {
            auto &sprite = _mmu->sprites[i];

            // Check if the sprite is on the current scanline
            if ((*LY < sprite.y) || (*LY >= sprite.y + sprite_height))
                continue;

            // Limit the number of sprites per scanline to 10
            if (spritesRendered >= 10)
                break;

            for (int x = 0; x < 8; x++)
            {
                int pixelX = sprite.x + x;
                if (pixelX < 0 || pixelX >= SCREEN_WIDTH)
                    continue;

                int spriteX = x;
                if (sprite.options.xFlip)
                    spriteX = 7 - x;
                int spriteY = *LY - sprite.y;
                if (sprite.options.yFlip)
                    spriteY = sprite_height - 1 - spriteY;

                int tile_num = sprite.tile & (LCDC->spriteSize ? 0xFE : 0xFF);
                int colour = 0;
                if (LCDC->spriteSize && spriteY >= 8)
                    colour = _mmu->tiles[tile_num + 1].pixels[spriteY - 8][spriteX];
                else
                    colour = _mmu->tiles[tile_num].pixels[spriteY][spriteX];

                if (colour == 0)
                    continue;
                if (sprite.options.renderPriority && rowPixels[pixelX])
                    continue;

                int pixelOffset = *LY * SCREEN_WIDTH + pixelX;
                if (sprite.colourPalette && pixelOffset >= 0 && pixelOffset < SCREEN_WIDTH * SCREEN_HEIGHT)
                {
                    _framebuffer[pixelOffset] = sprite.colourPalette[colour];
                }
            }

            spritesRendered++;
        }
    }

    void Ppu::reset()
    {
        LCDC = (Control *)&_mmu->_memory[0xff40];
        STAT = (Stat *)&_mmu->_memory[0xff41];
        SCY = &_mmu->_memory[0xff42];
        SCX = &_mmu->_memory[0xff43];
        LY = &_mmu->_memory[0xff44];
    }
}
