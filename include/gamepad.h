#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include <SDL.h>
#include <vector>

namespace gasyboy
{
    class Gamepad
    {
    private:
        bool _buttonSelected;
        bool _directionSelected;
        uint8_t _currentState;
        std::vector<bool> _directionKeys;
        std::vector<bool> _buttonKeys;

        enum Button
        {
            A,
            B,
            START,
            SELECT
        };

        enum Direction
        {
            UP,
            DOWN,
            LEFT,
            RIGHT
        };

    public:
        Gamepad();
        ~Gamepad() = default;

        void handleEvent();

        void setState(const uint8_t &value);

        uint8_t getState();

        static bool changedPalette;
    };
}

#endif