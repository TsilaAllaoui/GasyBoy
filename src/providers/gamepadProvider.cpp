#include "gamepadProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::unique_ptr<Gamepad> GamepadProvider::_gamepadInstance = nullptr;

        Gamepad &GamepadProvider::getInstance()
        {
            if (!_gamepadInstance)
            {
                _gamepadInstance = std::make_unique<Gamepad>();
            }
            return *_gamepadInstance;
        }

        void GamepadProvider::deleteInstance()
        {
            _gamepadInstance.reset(nullptr);
        }
    }
}