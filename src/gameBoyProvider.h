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
        static std::shared_ptr<GameBoy> _instance;

        static std::vector<uint8_t> _tetrisBytes;

    public:
        // Reset the current gameboy instance
        static void reset(const std::string &filePath, const bool &bootBios, const bool &debugMode = false);
        static void reset(const uint8_t *bytes, const size_t &romSize, const bool &bootBios, const bool &debugMode = false);

        // Get the gameboy instance
        static std::shared_ptr<GameBoy> getGameBoy(const bool &bootBios = true);

        // Destructor
        ~GameBoyProvider();

        GameBoyProvider() = delete;
        GameBoyProvider(const GameBoyProvider &) = delete;
        GameBoyProvider operator=(const GameBoyProvider &) = delete;
    };
}

#endif