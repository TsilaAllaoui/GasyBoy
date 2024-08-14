#include "gamepad.h"
#include "defs.h"
#include "SDL.h"

namespace gasyboy
{
    bool Gamepad::changedPalette = false;

    Gamepad::Gamepad()
        : _buttonSelected(true),
          _directionSelected(true),
          _currState(0)
    {
        _keys = std::vector<bool>(8, false);
    }

    void Gamepad::handleEvent()
    {
        SDL_Event event;

        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
                exit(0);

            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    exit(0);
                    break;

                case SDLK_a:
                    _keys[A] = true;
                    break;

                case SDLK_z:
                    _keys[B] = true;
                    break;

                case SDLK_RETURN:
                    _keys[SELECT] = true;
                    break;

                case SDLK_SPACE:
                    _keys[START] = true;
                    break;

                case SDLK_UP:
                case SDLK_o:
                    _keys[UP] = true;
                    break;

                case SDLK_DOWN:
                case SDLK_l:
                    _keys[DOWN] = true;
                    break;

                case SDLK_LEFT:
                case SDLK_k:
                    _keys[LEFT] = true;
                    break;

                case SDLK_RIGHT:
                case SDLK_m:
                    _keys[RIGHT] = true;
                    break;

                case SDLK_p:
                    changedPalette = true;
                }
            }

            if (event.type == SDL_KEYUP)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    exit(0);
                    break;

                case SDLK_a:
                    _keys[A] = false;
                    break;

                case SDLK_z:
                    _keys[B] = false;
                    break;

                case SDLK_RETURN:
                    _keys[SELECT] = false;
                    break;

                case SDLK_SPACE:
                    _keys[START] = false;
                    break;

                case SDLK_UP:
                case SDLK_o:
                    _keys[UP] = false;
                    break;

                case SDLK_DOWN:
                case SDLK_l:
                    _keys[DOWN] = false;
                    break;

                case SDLK_LEFT:
                case SDLK_k:
                    _keys[LEFT] = false;
                    break;

                case SDLK_RIGHT:
                case SDLK_m:
                    _keys[RIGHT] = false;
                    break;
                }
            }
        }
    }

    void Gamepad::setState(uint8_t value)
    {
        _buttonSelected = ((value & 0x20) == 0x20);
        _directionSelected = ((value & 0x10) == 0x10);
    }

    uint8_t Gamepad::getState()
    {
        _currState = 0xCF;

        if (!_buttonSelected)
        {
            if (_keys[A])
                _currState &= ~(1 << 0);

            if (_keys[B])
                _currState &= ~(1 << 1);

            if (_keys[SELECT])
                _currState &= ~(1 << 2);

            if (_keys[START])
                _currState &= ~(1 << 3);

            _currState |= 0x10;
        }

        else if (!_directionSelected)
        {
            if (_keys[RIGHT])
                _currState &= ~(1 << 0);

            if (_keys[LEFT])
                _currState &= ~(1 << 1);

            if (_keys[UP])
                _currState &= ~(1 << 2);

            if (_keys[DOWN])
                _currState &= ~(1 << 3);

            _currState |= 0x20;
        }

        return _currState;
    }
}