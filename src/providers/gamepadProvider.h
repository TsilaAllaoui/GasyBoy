#ifndef __GAMEPAD_PROVIDER_H__
#define __GAMEPAD_PROVIDER_H__

#include <memory>
#include "gamepad.h"

namespace gasyboy
{
    namespace provider
    {
        class GamepadProvider
        {
        private:
            static std::unique_ptr<Gamepad> _gamepadInstance;

            GamepadProvider() = default;

        public:
            ~GamepadProvider() = default;

            GamepadProvider(const GamepadProvider &) = delete;
            GamepadProvider &operator=(const GamepadProvider &) = delete;

            static Gamepad &getInstance();

            static void deleteInstance();
        };
    }
}

#endif