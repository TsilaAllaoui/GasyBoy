#include "gamepadProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::shared_ptr<Gamepad> GamepadProvider::_gamepadInstance = nullptr;

        std::shared_ptr<Gamepad> GamepadProvider::getInstance()
        {
            if (!_gamepadInstance)
            {
                _gamepadInstance = std::make_shared<Gamepad>();
            }
            return _gamepadInstance;
        }

        void GamepadProvider::deleteInstance()
        {
            _gamepadInstance.reset();
        }
    }
}