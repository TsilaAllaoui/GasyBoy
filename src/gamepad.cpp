#include "gamepad.h"
#include "defs.h"
#include "SDL.h"

namespace gasyboy
{
    Gamepad::Gamepad()
        : _buttonSelected(true),
          _directionSelected(true),
          _changedPalette(false),
          _state(0xF)
    {
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

                case SDLK_UP:
                case SDLK_RETURN:
                    _state.reset(Button::SELECT_OR_UP);
                    break;

                case SDLK_DOWN:
                case SDLK_SPACE:
                    _state.reset(Button::START_OR_DOWN);
                    break;

                case SDLK_RIGHT:
                case SDLK_a:
                    _state.reset(Button::A_OR_RIGHT);
                    break;

                case SDLK_LEFT:
                case SDLK_z:
                    _state.reset(Button::B_OR_LEFT);
                    break;

                case SDLK_p:
                    _changedPalette = true;
                }
            }

            if (event.type == SDL_KEYUP)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    exit(0);
                    break;

                case SDLK_UP:
                case SDLK_RETURN:
                    _state.set(Button::SELECT_OR_UP);
                    break;

                case SDLK_DOWN:
                case SDLK_SPACE:
                    _state.set(Button::START_OR_DOWN);
                    break;

                case SDLK_RIGHT:
                case SDLK_a:
                    _state.set(Button::A_OR_RIGHT);
                    break;

                case SDLK_LEFT:
                case SDLK_z:
                    _state.set(Button::B_OR_LEFT);
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
        uint8_t value = 0xC0;

        value |= static_cast<uint8_t>(_state.to_ulong());

        return value;
    }

    void Gamepad::setChangePalette(const bool &value)
    {
        _changedPalette = value;
    }

    bool Gamepad::getChangePalette()
    {
        return _changedPalette;
    }
}