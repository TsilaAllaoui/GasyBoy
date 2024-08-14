#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include <bitset>

namespace gasyboy
{
    class Gamepad
    {
        // To check what type of button the emulator want to check
        bool _buttonSelected;
        bool _directionSelected;

        // The current button state
        std::bitset<4> _state;

        // To change palette color
        bool _changedPalette;

    public:
        // Constructor
        Gamepad();

        // Handle key press/release events
        void handleEvent();

        // Set the selected type of button
        void setState(uint8_t value);

        // Get pressed buttons
        uint8_t getState();

        // Get/Set palette color if it's changed or not
        void setChangePalette(const bool &value);
        bool getChangePalette();

        // Enum for the type of button
        enum Button
        {
            A_OR_RIGHT = 0,
            B_OR_LEFT = 1,
            SELECT_OR_UP = 2,
            START_OR_DOWN = 3
        };
    };
}

#endif