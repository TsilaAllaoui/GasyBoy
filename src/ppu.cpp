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
            STAT->modeFlag = PpuMode::HBLANK;
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
                updateLY();
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
                updateLY();
                if (*LY > 153)
                {
                    _canRender = true;
                    // *LY = 0;
                    setMode(PpuMode::OAM_SEARCH);
                }
            }
            break;
        }
    }

    void Ppu::setMode(PpuMode mode)
    {
        STAT->modeFlag = static_cast<uint8_t>(mode);

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

    void Ppu::updateLY()
    {
        (*LY)++;
        if (*LY == 144)
        {
            setMode(PpuMode::VBLANK);
            _interruptManager->requestInterrupt(InterruptManager::InterruptType::VBlank);
        }

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
    }

    void Ppu::renderScanLines()
    {
        // Initialize the rowPixels array to false for all 160 pixels.
        bool rowPixels[160] = {0};

        // If the Background Enable bit is set, render BG and window.
        if (LCDC->bgDisplay)
        {
            renderScanLineBackground(rowPixels);

            // Pass rowPixels to window rendering so nonzero window pixels are marked.
            if (LCDC->windowEnable)
            {
                renderScanLineWindow(rowPixels);
            }
        }

        // Render sprites (they use rowPixels to check BG priority).
        if (LCDC->spriteDisplayEnable)
        {
            renderScanLineSprites(rowPixels);
        }
    }

    void Ppu::renderScanLineBackground(bool *rowPixels)
    {
        // 1. Base tilemap address
        uint16_t tileMapBase = 0x9800;
        if (LCDC->bgDisplaySelect)
            tileMapBase += 0x400; // switch to 9C00 region

        // 2. Compute the absolute y in the 256x256 BG, then wrap to 0–255
        uint8_t y = *LY + *SCY;
        //    tileRow = which of the 32 tile rows (0–31)
        uint8_t tileRow = (y >> 3) & 31;
        //    line inside that tile (0–7)
        uint8_t tileLine = y & 7;

        // 3. Similarly, find the starting tile column (0–31), plus the pixel offset within that tile (0–7).
        uint8_t xOffset = *SCX & 7;
        uint8_t tileColumn = (*SCX >> 3) & 31;

        // 4. Start writing into the framebuffer at this scanline
        int pixelOffset = *LY * SCREEN_WIDTH;
        int screenX = 0; // which x pixel on this scanline?

        // 5. Each tile is 8 pixels wide. We need ~21 tiles to cover 160 px.
        //    We'll fetch the tile number from tileMap, then draw up to 8 pixels from it.
        //    Then move on to the next tile, wrapping tileColumn with &31.
        for (int i = 0; i < 21 && screenX < 160; i++)
        {
            // Address in the BG tilemap for [tileRow, tileColumn]
            uint16_t tileMapAddr = tileMapBase + tileRow * 32 + tileColumn;

            // Read the tile index from VRAM
            int tileIndex = _mmu->readRam(tileMapAddr);
            // If in signed addressing mode (bgWindowDataSelect=0) and tileIndex<128, adjust by +256
            if (!LCDC->bgWindowDataSelect && tileIndex < 128)
                tileIndex += 256;

            // 6. Draw up to 8 pixels from this tile, starting at xOffset
            for (; xOffset < 8 && screenX < 160; xOffset++)
            {
                if (pixelOffset >= SCREEN_WIDTH * SCREEN_HEIGHT)
                    return;

                int colorIndex = _mmu->tiles[tileIndex].pixels[tileLine][xOffset];
                _framebuffer[pixelOffset + screenX] = _mmu->palette_BGP[colorIndex];

                // Mark rowPixels if BG pixel is nonzero
                if (colorIndex > 0)
                    rowPixels[screenX] = true;

                screenX++;
            }

            // Move on to next tile in this row
            xOffset = 0;
            tileColumn = (tileColumn + 1) & 31; // wrap horizontally
        }
    }

    // Updated renderScanLineWindow: now accepts rowPixels so that any nonzero
    // window pixel is marked (hiding BG-priority sprites).
    void Ppu::renderScanLineWindow(bool *rowPixels)
    {
        // Only render the window if LY has reached WY and WX is valid.
        if (*LY < *WY || *WX >= 167)
            return;

        // Reset the window internal line counter on the very first window line.
        if (*LY == *WY)
            windowLineCounter = 0;

        uint8_t wx = *WX;
        uint16_t baseAddress = LCDC->windowDisplaySelect ? 0x9C00 : 0x9800;

        // Use the internal window counter (which starts at 0 when the window first appears)
        int tileY = windowLineCounter / 8;
        int pixelYInTile = windowLineCounter % 8;
        uint16_t tileMapRowAddr = baseAddress + tileY * 32;

        int pixelOffset = *LY * SCREEN_WIDTH;
        int startX = wx - 7;

        // Draw up to 21 tiles covering the screen horizontally
        for (int tileX = 0; tileX < 21; tileX++)
        {
            uint16_t tileAddress = tileMapRowAddr + tileX;
            int tileIndex = _mmu->readRam(tileAddress);
            if (!LCDC->bgWindowDataSelect && tileIndex < 128)
                tileIndex += 256;

            for (int x = 0; x < 8; x++)
            {
                if (pixelOffset >= SCREEN_WIDTH * SCREEN_HEIGHT)
                    return;

                int windowPixelX = startX + tileX * 8 + x;
                if (windowPixelX >= SCREEN_WIDTH)
                    break;
                if (windowPixelX < 0)
                    continue;

                int colorIndex = _mmu->tiles[tileIndex].pixels[pixelYInTile][x];
                int frameIndex = pixelOffset + windowPixelX;
                _framebuffer[frameIndex] = _mmu->palette_BGP[colorIndex];
                if (colorIndex > 0)
                    rowPixels[windowPixelX] = true;
            }
        }

        // Increment only if the window was drawn on this line.
        windowLineCounter++;
    }

    void Ppu::renderScanLineSprites(bool *rowPixels)
    {
        int spriteHeight = LCDC->spriteSize ? 16 : 8;
        int spritesRendered = 0;

        // For each x coordinate on the scanline, store the sprite.x value of
        // the sprite that currently occupies that pixel. Initialize to a high value.
        int spriteXPriority[SCREEN_WIDTH];
        for (int i = 0; i < SCREEN_WIDTH; i++)
        {
            spriteXPriority[i] = 256; // 256 is outside valid range (0-159) so it means "no sprite"
        }

        // Process all 40 sprites from OAM.
        for (int i = 0; i < 40; i++)
        {
            auto &sprite = _mmu->sprites[i];

            // Check if the current scanline (*LY) is within the sprite's vertical bounds.
            if (*LY < sprite.y || *LY >= sprite.y + spriteHeight)
                continue;

            // Limit to 10 sprites per scanline.
            if (spritesRendered >= 10)
                break;

            // Calculate the vertical offset within the sprite.
            int spriteY = *LY - sprite.y;
            if (sprite.options.yFlip)
                spriteY = spriteHeight - 1 - spriteY;

            // In 8x16 mode, force tile index to be even.
            int baseTileIndex = LCDC->spriteSize ? (sprite.tile & 0xFE) : sprite.tile;
            int tileIndex = baseTileIndex;

            // For 8x16 sprites, if in the bottom half, select the second tile.
            if (LCDC->spriteSize && spriteY >= 8)
            {
                tileIndex += 1;
                spriteY -= 8; // Adjust offset to index into the correct row of the second tile.
            }

            // Process each of the 8 horizontal pixels in the sprite.
            for (int x = 0; x < 8; x++)
            {
                int pixelX = sprite.x + x;
                if (pixelX < 0 || pixelX >= SCREEN_WIDTH)
                    continue;

                // Check object priority: if a sprite pixel is already drawn at this screen x,
                // only override if the current sprite's x coordinate is lower (further left).
                if (sprite.x >= spriteXPriority[pixelX])
                    continue;

                // Apply horizontal flip if needed.
                int spriteX = sprite.options.xFlip ? 7 - x : x;

                // Fetch the color index from the tile pixel data.
                int colour = _mmu->tiles[tileIndex].pixels[spriteY][spriteX];
                if (colour == 0)
                    continue; // Transparent pixel.

                // Check background priority (bit 7): if set and the BG (or window) pixel is nonzero, skip drawing.
                if (sprite.options.renderPriority && rowPixels[pixelX])
                    continue;

                int pixelOffset = *LY * SCREEN_WIDTH + pixelX;
                if (sprite.colourPalette && pixelOffset >= 0 && pixelOffset < SCREEN_WIDTH * SCREEN_HEIGHT)
                {
                    _framebuffer[pixelOffset] = sprite.colourPalette[colour];
                    // Record the x coordinate of the sprite that drew this pixel.
                    spriteXPriority[pixelX] = sprite.x;
                }
            }
            spritesRendered++;
        }
    }

    void Ppu::refresh()
    {
        auto ly = *LY;
        *LY = 0;

        for (size_t i = 0; i < 144; i++)
        {
            renderScanLines();
            (*LY) += 1;
        }

        *LY = ly;
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
