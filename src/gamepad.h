#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include <vector>

namespace gasyboy
{
    class Gamepad
    {
        bool _buttonSelected;
        bool _directionSelected;
        uint8_t _currState;
        std::vector<bool> _keys;
        enum
        {
            A,
            B,
            START,
            SELECT,
            UP,
            DOWN,
            LEFT,
            RIGHT
        };

    public:
        Gamepad();

        void handleEvent();

        void setState(uint8_t value);

        uint8_t getState();

        static bool changedPalette;
    };
}

#endif