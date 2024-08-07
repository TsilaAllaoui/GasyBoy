#include "gamepad.h"
#include "defs.h"

namespace gasyboy
{
    bool Gamepad::changedPalette = false;

    Gamepad::Gamepad()
    {
        _buttonSelected = true;
        _directionSelected = true;
        _currentState = 0;
        _directionKeys = std::vector<bool>(4, false);
        _buttonKeys = std::vector<bool>(4, false);
    }

    void Gamepad::handleEvent()
    {
        SDL_Event event;

        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
                exit(ExitState::MANUAL_STOP);

            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    exit(ExitState::MANUAL_STOP);
                    break;

                case SDLK_a:
                    _buttonKeys[Button::A] = true;
                    break;

                case SDLK_z:
                    _buttonKeys[Button::B] = true;
                    break;

                case SDLK_RETURN:
                    _buttonKeys[Button::SELECT] = true;
                    break;

                case SDLK_SPACE:
                    _buttonKeys[Button::START] = true;
                    break;

                case SDLK_UP:
                case SDLK_o:
                    _directionKeys[Direction::UP] = true;
                    break;

                case SDLK_DOWN:
                case SDLK_l:
                    _directionKeys[Direction::DOWN] = true;
                    break;

                case SDLK_LEFT:
                case SDLK_k:
                    _directionKeys[Direction::LEFT] = true;
                    break;

                case SDLK_RIGHT:
                case SDLK_m:
                    _directionKeys[Direction::RIGHT] = true;
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
                    exit(ExitState::MANUAL_STOP);
                    break;

                case SDLK_a:
                    _buttonKeys[Button::A] = false;
                    break;

                case SDLK_z:
                    _buttonKeys[Button::B] = false;
                    break;

                case SDLK_RETURN:
                    _buttonKeys[Button::SELECT] = false;
                    break;

                case SDLK_SPACE:
                    _buttonKeys[Button::START] = false;
                    break;

                case SDLK_UP:
                case SDLK_o:
                    _directionKeys[Direction::UP] = false;
                    break;

                case SDLK_DOWN:
                case SDLK_l:
                    _directionKeys[Direction::DOWN] = false;
                    break;

                case SDLK_LEFT:
                case SDLK_k:
                    _directionKeys[Direction::LEFT] = false;
                    break;

                case SDLK_RIGHT:
                case SDLK_m:
                    _directionKeys[Direction::RIGHT] = false;
                    break;
                }
            }
        }
    }

    void Gamepad::setState(const uint8_t &value)
    {
        _buttonSelected = ((value & 0x20) == 0x20);
        _directionSelected = ((value & 0x10) == 0x10);
    }

    uint8_t Gamepad::getState()
    {
        _currentState = 0xCF;

        auto &keys = _buttonSelected ? _buttonKeys : _directionKeys;

        if (keys[_buttonSelected ? Button::A : Direction::UP])
            _currentState &= ~(1 << 0);

        if (keys[_buttonSelected ? Button::B : Direction::DOWN])
            _currentState &= ~(1 << 1);

        if (keys[_buttonSelected ? Button::START : Direction::LEFT])
            _currentState &= ~(1 << 2);

        if (keys[_buttonSelected ? Button::SELECT : Direction::RIGHT])
            _currentState &= ~(1 << 3);

        _currentState |= 0x10;

        return _currentState;
    }
}