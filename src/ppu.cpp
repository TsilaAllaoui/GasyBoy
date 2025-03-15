#include "interruptManagerProvider.h"
#include "registersProvider.h"
#include "mmuProvider.h"
#include "ppu.h"

namespace gasyboy
{
    Ppu::Ppu()
        : _mmu(provider::MmuProvider::getInstance()),
          _registers(provider::RegistersProvider::getInstance()),
          _interruptManager(provider::InterruptManagerProvider::getInstance())
    {
        _control = (Control *)&_mmu._memory[0xff40];
        _lcdStat = (Stat *)&_mmu._memory[0xff41];
        _scrollY = &_mmu._memory[0xff42];
        _scrollX = &_mmu._memory[0xff43];
        _scanline = &_mmu._memory[0xff44];
    }

    void Ppu::step(const int &cycle)
    {
        _modeClock += cycle;

        if (!_control->lcdEnable)
        {
            _mode = 0;
            if (_modeClock >= 70224)
                _modeClock -= 70224;
            return;
        }

        switch (_mode)
        {
        case 0: // HBLANK
            if (_modeClock >= 204)
            {
                _modeClock -= 204;
                _mode = 2;

                *_scanline += 1;
                compareLyAndLyc();

                if (*_scanline == 144)
                {
                    _mode = 1;
                    _canRender = true;
                    _interruptManager.requestInterrupt(InterruptManager::InterruptType::VBlank);
                    if (_lcdStat->vblankInterrupt)
                        _interruptManager.requestInterrupt(InterruptManager::InterruptType::LCDStat);
                }
                else if (_lcdStat->oamInterrupt)
                {
                    _interruptManager.requestInterrupt(InterruptManager::InterruptType::LCDStat);
                }

                _mmu.writeRam(0xff41, (_mmu.readRam(0xff41) & 0xFC) | (_mode & 3));
            }
            break;
        case 1: // VBLANK
            if (_modeClock >= 456)
            {
                _modeClock -= 456;
                *_scanline += 1;
                compareLyAndLyc();
                if (*_scanline == 153)
                {
                    *_scanline = 0;
                    _mode = 2;
                    _mmu.writeRam(0xff41, (_mmu.readRam(0xff41) & 0xFC) | (_mode & 3));
                    if (_lcdStat->oamInterrupt)
                        _interruptManager.requestInterrupt(InterruptManager::InterruptType::LCDStat);
                }
            }
            break;
        case 2: // OAM
            if (_modeClock >= 80)
            {
                _modeClock -= 80;
                _mode = 3;
                _mmu.writeRam(0xff41, (_mmu.readRam(0xff41) & 0xFC) | (_mode & 3));
            }
            break;
        case 3: // VRAM
            if (_modeClock >= 172)
            {
                _modeClock -= 172;
                _mode = 0;
                renderScanLines();
                _mmu.writeRam(0xff41, (_mmu.readRam(0xff41) & 0xFC) | (_mode & 3));
                if (_lcdStat->hblankInterrupt)
                    _interruptManager.requestInterrupt(InterruptManager::InterruptType::LCDStat);
            }
            break;
        }
    }

    void Ppu::compareLyAndLyc()
    {
        uint8_t lyc = _mmu.readRam(0xFF45);
        _lcdStat->coincidenceFlag = int(lyc == *_scanline);
        if (lyc == *_scanline && _lcdStat->coincidenceInterrupt)
            _interruptManager.requestInterrupt(InterruptManager::InterruptType::LCDStat);
    }

    void Ppu::renderScanLines()
    {
        bool rowPixels[160] = {0};
        renderScanLineBackground(rowPixels);
        renderScanLineWindow();
        renderScanLineSprites(rowPixels);
    }

    void Ppu::renderScanLineBackground(bool *rowPixels)
    {
        uint16_t address = 0x9800;
        if (_control->bgDisplaySelect)
            address += 0x400;

        uint16_t startRowAddress = address + ((*_scrollY + *_scanline) / 8) * 32;
        uint16_t endRowAddress = startRowAddress + 32;
        address = startRowAddress + ((*_scrollX) >> 3);

        int x = *_scrollX & 7;
        int y = (*_scanline + *_scrollY) & 7;
        int pixelOffset = *_scanline * SCREEN_WIDTH;
        int pixel = 0;
        for (uint16_t i = 0; i < 21; i++)
        {
            uint16_t tile_address = address + i;
            if (tile_address >= endRowAddress)
                tile_address = startRowAddress + (tile_address % 32);

            int tile = _mmu.readRam(tile_address);
            if (!_control->bgWindowDataSelect && tile < 128)
                tile += 256;

            for (; x < 8 && pixel < 160; x++)
            {
                if (pixelOffset >= SCREEN_WIDTH * SCREEN_HEIGHT)
                    return;
                int colour = _mmu.tiles[tile].pixels[y][x];
                _framebuffer[pixelOffset++] = _mmu.palette_BGP[colour];
                if (colour > 0)
                    rowPixels[pixel] = true;
                pixel++;
            }
            x = 0;
        }
    }

    void Ppu::renderScanLineWindow()
    {
        if (!_control->windowEnable)
            return;
        uint8_t windowY = _mmu.readRam(0xFF4A);
        if (windowY > *_scanline)
            return;

        uint16_t address = 0x9800;
        if (_control->windowDisplaySelect)
            address += 0x400;
        uint8_t windowX = _mmu.readRam(0xFF4B) - 7;
        int y = (*_scanline - windowY) & 7;
        int pixelOffset = *_scanline * SCREEN_WIDTH;
        address += ((*_scanline - windowY) / 8) * 32;

        for (uint16_t tileX = 0; tileX < 21; tileX++)
        {
            uint16_t tile_address = address + tileX;
            int tile = _mmu.readRam(tile_address);
            if (!_control->bgWindowDataSelect && tile < 128)
                tile += 256;

            for (int x = 0; x < 8; x++)
            {
                int windowPixelX = windowX + tileX * 8 + x;
                if (windowPixelX < 0 || windowPixelX >= SCREEN_WIDTH)
                    continue;
                int colour = _mmu.tiles[tile].pixels[y][x];
                if (pixelOffset + windowPixelX >= SCREEN_WIDTH * SCREEN_HEIGHT)
                    return;
                _framebuffer[pixelOffset + windowPixelX] = _mmu.palette_BGP[colour];
            }
        }
    }

    void Ppu::renderScanLineSprites(bool *rowPixels)
    {
        int sprite_height = _control->spriteSize ? 16 : 8;
        int spritesRendered = 0;

        for (int i = 0; i < 40; i++)
        {
            auto &sprite = _mmu.sprites[i];

            // Check if the sprite is on the current scanline
            if ((*_scanline < sprite.y) || (*_scanline >= sprite.y + sprite_height))
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
                int spriteY = *_scanline - sprite.y;
                if (sprite.options.yFlip)
                    spriteY = sprite_height - 1 - spriteY;

                int tile_num = sprite.tile & (_control->spriteSize ? 0xFE : 0xFF);
                int colour = 0;
                if (_control->spriteSize && spriteY >= 8)
                    colour = _mmu.tiles[tile_num + 1].pixels[spriteY - 8][spriteX];
                else
                    colour = _mmu.tiles[tile_num].pixels[spriteY][spriteX];

                if (colour == 0)
                    continue;
                if (sprite.options.renderPriority && rowPixels[pixelX])
                    continue;

                int pixelOffset = *_scanline * SCREEN_WIDTH + pixelX;
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
        _control = (Control *)&_mmu._memory[0xff40];
        _lcdStat = (Stat *)&_mmu._memory[0xff41];
        _scrollY = &_mmu._memory[0xff42];
        _scrollX = &_mmu._memory[0xff43];
        _scanline = &_mmu._memory[0xff44];
    }
}
