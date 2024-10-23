#include "ppu.h"

namespace gasyboy
{
    Ppu::Ppu(Registers &registers, InterruptManager &interruptManager, Mmu &mmu)
        : _mmu(mmu),
          _registers(registers),
          _interruptManager(interruptManager)

    {
        _control = (Control *)_mmu.ramCellptr(0xff40);
        _lcdStat = (Stat *)_mmu.ramCellptr(0xff41);
        _scrollY = (uint8_t *)_mmu.ramCellptr(0xff42);
        _scrollX = (uint8_t *)_mmu.ramCellptr(0xff43);
        _scanline = (uint8_t *)_mmu.ramCellptr(0xff44);
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
                    {
                        _interruptManager.requestInterrupt(InterruptManager::InterruptType::LCDStat);
                    }
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

        // Calculate starting position within the tile map
        uint16_t startRowAddress = address + ((*_scrollY + *_scanline) / 8) * 32;
        uint16_t endRowAddress = startRowAddress + 32; // End of the current row in the tile map
        address = startRowAddress + (*_scrollX >> 3);  // Adjust for horizontal scrolling

        int x = *_scrollX & 7;                       // Initial pixel offset within the tile
        int y = (*_scanline + *_scrollY) & 7;        // Vertical pixel position within the tile
        int pixelOffset = *_scanline * SCREEN_WIDTH; // Start position in the framebuffer

        int pixel = 0; // Screen pixel position
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
                    return; // Prevent out-of-bounds access

                int colour = _mmu.tiles[tile].pixels[y][x];
                _framebuffer[pixelOffset++] = _mmu.palette_BGP[colour];
                if (colour > 0)
                    rowPixels[pixel] = true;
                pixel++;
            }
            x = 0; // Reset horizontal position for the next tile
        }
    }

    void Ppu::renderScanLineWindow()
    {
        // Check if window is enabled
        if (!_control->windowEnable)
        {
            return;
        }

        // Get the window Y position from 0xFF4A register
        uint8_t windowY = _mmu.readRam(0xFF4A);

        // If the current scanline is above the window's Y position, do nothing
        if (windowY > *_scanline)
        {
            return;
        }

        // Determine the tile map address for the window
        uint16_t address = 0x9800;
        if (_control->windowDisplaySelect)
        {
            address += 0x400;
        }

        // Calculate the starting address of the current window row
        uint8_t windowX = _mmu.readRam(0xFF4B) - 7;  // Get the window X position from 0xFF4B register
        int y = (*_scanline - windowY) & 7;          // Vertical pixel position within the tile
        int pixelOffset = *_scanline * SCREEN_WIDTH; // Start position in the framebuffer

        // Calculate the starting tile map address
        address += ((*_scanline - windowY) / 8) * 32;

        // Iterate through each tile in the current scanline of the window
        for (uint16_t tileX = 0; tileX < 21; tileX++)
        {
            // Read the tile index from VRAM
            uint16_t tile_address = address + tileX;
            int tile = _mmu.readRam(tile_address);

            // Adjust for tile index based on data select mode
            if (!_control->bgWindowDataSelect && tile < 128)
            {
                tile += 256;
            }

            // Iterate through each pixel in the tile
            for (int x = 0; x < 8; x++)
            {
                int windowPixelX = windowX + tileX * 8 + x;
                if (windowPixelX < 0 || windowPixelX >= SCREEN_WIDTH)
                {
                    continue; // Skip if pixel is outside the screen bounds
                }

                // Calculate the color of the pixel
                int colour = _mmu.tiles[tile].pixels[y][x];

                // Ensure pixelOffset is within bounds of the framebuffer
                if (pixelOffset + windowPixelX >= SCREEN_WIDTH * SCREEN_HEIGHT)
                {
                    return; // Prevent out-of-bounds access
                }

                // Write the color to the framebuffer
                _framebuffer[pixelOffset + windowPixelX] = _mmu.palette_BGP[colour];
            }
        }
    }

    void Ppu::renderScanLineSprites(bool *rowPixels)
    {
        int sprite_height = _control->spriteSize ? 16 : 8;

        bool visible_sprites[40] = {false};
        int sprite_row_count = 0;

        // Determine which sprites are visible on the current scanline
        for (int i = 0; i < 40; i++)
        {
            auto &sprite = _mmu.sprites[i];

            // Check if the sprite is on the current scanline
            if ((*_scanline >= sprite.y) && (*_scanline < (sprite.y + sprite_height)))
            {
                visible_sprites[i] = true;
                sprite_row_count++;
            }

            // Limit to 10 sprites per scanline
            if (sprite_row_count >= 10)
                break;
        }

        // Render the visible sprites
        for (int i = 39; i >= 0; i--)
        {
            if (!visible_sprites[i])
                continue;

            auto &sprite = _mmu.sprites[i];

            // Skip sprites that are completely off-screen horizontally
            if (sprite.x >= 160 || sprite.x < -7)
                continue;

            // Calculate the y-position of the sprite line to draw
            int pixel_y = *_scanline - sprite.y;
            if (sprite.options.yFlip)
                pixel_y = (sprite_height - 1) - pixel_y;

            // Render the sprite line
            for (int x = 0; x < 8; x++)
            {
                int tile_num = sprite.tile & (_control->spriteSize ? 0xFE : 0xFF);

                int x_temp = sprite.x + x;
                if (x_temp < 0 || x_temp >= 160)
                    continue;

                // Calculate pixelOffset and ensure it's within the valid range
                int pixelOffset = *_scanline * SCREEN_WIDTH + x_temp;
                if (pixelOffset < 0 || pixelOffset >= SCREEN_WIDTH * SCREEN_HEIGHT)
                    continue;

                // Flip horizontally
                int pixel_x = sprite.options.xFlip ? 7 - x : x;

                // Determine the color from the appropriate tile and pixel
                int colour = 0;
                if (_control->spriteSize && pixel_y >= 8)
                    colour = _mmu.tiles[tile_num + 1].pixels[pixel_y - 8][pixel_x];
                else
                    colour = _mmu.tiles[tile_num].pixels[pixel_y][pixel_x];

                // Skip transparent pixels
                if (colour == 0)
                    continue;

                // Render the pixel if it has higher priority or the background pixel is transparent
                if (!rowPixels[x_temp] || !sprite.options.renderPriority)
                    _framebuffer[pixelOffset] = sprite.colourPalette[colour];
            }
        }
    }

}
