#ifndef _UTILS_H_
#define _UTILS_H_

#include "cartridge.h"

namespace gasyboy
{
    namespace utils
    {
        // Convert Uint8 to CartridgeType
        Cartridge::CartridgeType uint8ToCartridgeType(const uint8_t &value);
    }
}

#endif