#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include <bitset>

namespace gasyboy
{
    class Gamepad
    {
        // To check what type of button the emulator want to check
        static bool _buttonSelected;

        // The current button state
        std::bitset<8> _state;

        // To change palette color
        bool _changedPalette;

    public:
        // Constructor
        Gamepad();

        // Reset
        void reset();

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
            A = 0,
            B = 1,
            SELECT = 2,
            START = 3,
            RIGHT = 4,
            LEFT = 5,
            UP = 6,
            DOWN = 7
        };

        // To check if button or d-pad is selected
        static bool isButtonSelected();
    };
}

#endif