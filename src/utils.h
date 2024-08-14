#ifndef _UTILS_H_
#define _UTILS_H_

#include "cartridge.h"
#include <string>

namespace gasyboy
{
    namespace utils
    {
        // Convert Uint8 to CartridgeType
        Cartridge::CartridgeType uint8ToCartridgeType(const uint8_t &value);

        class XToString
        {
        public:
            std::string toString(const Cartridge::CartridgeType &cartridgeType);
        };
    }
}

#endif