#ifndef __GAMEBOY_PROVIDER_H__
#define __GAMEBOY_PROVIDER_H__

#include "gameboy.h"
#include <memory>
#include <string>
#include <vector>

namespace gasyboy
{
    class GameBoyProvider
    {
        static std::unique_ptr<GameBoy> _gameboyInstance;

        static std::vector<uint8_t> _tetrisBytes;

    public:
        GameBoyProvider() = default;
        ~GameBoyProvider() = default;

        static GameBoy &getInstance();

        static void deleteInstance();

        // // Reset the current gameboy instance
        // static void reset(const std::string &filePath, const bool &bootBios, const bool &debugMode = false);
        // static void reset(const uint8_t *bytes, const size_t &romSize, const bool &bootBios, const bool &debugMode = false);

        GameBoyProvider(const GameBoyProvider &) = delete;
        GameBoyProvider operator=(const GameBoyProvider &) = delete;
    };
}

#endif