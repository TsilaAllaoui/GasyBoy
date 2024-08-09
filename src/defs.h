#ifndef _DEFS_H_
#define _DEFS_H_

namespace gasyboy
{
#define MAXCYCLE 70224
#define SCALE 2
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define VRAM_WIDTH 128
#define VRAM_HEIGHT 192
#define BG_MAP_WIDTH 256
#define BG_MAP_HEIGTH 256
#define SPEED 3.6
#define MEM_ACCESS_READ 0
#define MEM_ACCESS_WRITE 1

    enum ExitState
    {
        SUCCESS = 0,
        CRITICAL_ERROR = 1,
        MANUAL_STOP = 2
    };
}

#endif
