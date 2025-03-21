#ifndef _DEFS_H_
#define _DEFS_H_

namespace gasyboy
{
#define MAXCYCLE 69905
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
#define FPS 60

    enum ExitState
    {
        SUCCESS = 0,
        CRITICAL_ERROR = 1,
        MANUAL_STOP = 2
    };
}

const uint8_t instructionTicks[256] = {
    4, 12, 8, 8, 4, 4, 8, 4, 20, 8, 8, 8, 4, 4, 8, 4,      // 0x0_
    4, 12, 8, 8, 4, 4, 8, 4, 12, 8, 8, 8, 4, 4, 8, 4,      // 0x1_
    0, 12, 8, 8, 4, 4, 8, 4, 0, 8, 8, 8, 4, 4, 8, 4,       // 0x2_
    0, 12, 8, 8, 12, 12, 12, 4, 0, 8, 8, 8, 4, 4, 8, 4,    // 0x3_
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,        // 0x4_
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,        // 0x5_
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,        // 0x6_
    8, 8, 8, 8, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4,        // 0x7_
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,        // 0x8_
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,        // 0x9_
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,        // 0xa_
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,        // 0xb_
    0, 12, 0, 16, 0, 16, 8, 16, 0, 16, 0, 0, 0, 24, 8, 16, // 0xc_
    0, 12, 0, 0, 0, 16, 8, 16, 0, 16, 0, 0, 0, 0, 8, 16,   // 0xd_
    12, 12, 8, 0, 0, 16, 8, 16, 16, 4, 16, 0, 0, 0, 8, 16, // 0xe_
    12, 12, 8, 4, 0, 16, 8, 16, 12, 8, 16, 4, 0, 0, 8, 16  // 0xf_
};
const uint8_t extendedInstructionTicks[256] = {
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x0_
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x1_
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x2_
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x3_
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x4_
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x5_
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x6_
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 0x7_
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x8_
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x9_
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0xa_
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0xb_
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0xc_
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0xd_
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0xe_
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8  // 0xf_
};

#endif
