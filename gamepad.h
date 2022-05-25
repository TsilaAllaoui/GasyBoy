#pragma once

#include "defs.h"
#include <iostream>

class Gamepad
{
    private:
        bool buttonSelected;
        bool directionSelected;
        uint8_t currState;
        bool keys[8];
        enum {A, B, START, SELECT, UP, DOWN, LEFT, RIGHT};

    public:
        Gamepad();
        ~Gamepad(){};
        void handleEvent();
        void setState(uint8_t value);
        uint8_t getState();
};