#include "gamepad.h"
#include "defs.h"
// #include "SDL.h"
#include <SDL2/SDL.h>
#ifdef EMSCRIPTEN
#include <SDL2/SDL.h>
#else
#endif

namespace gasyboy
{
    bool Gamepad::_buttonSelected = false;

    Gamepad::Gamepad()
        : _changedPalette(false),
          _state(0xFF)
    {
    }

    void Gamepad::reset()
    {
        _changedPalette = false;
        _state = 0xFF;
    }

    void Gamepad::handleEvent()
    {
        SDL_Event event;

        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
            {
                exit(ExitState::MANUAL_STOP);
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    exit(ExitState::MANUAL_STOP);
                    break;

                case SDLK_UP:
                    _state.reset(Button::UP);
                    break;
                case SDLK_RETURN:
                    _state.reset(Button::SELECT);
                    break;

                case SDLK_DOWN:
                    _state.reset(Button::DOWN);
                    break;
                case SDLK_SPACE:
                    _state.reset(Button::START);
                    break;

                case SDLK_RIGHT:
                    _state.reset(Button::RIGHT);
                    break;
                case SDLK_a:
                    _state.reset(Button::A);
                    break;

                case SDLK_LEFT:
                    _state.reset(Button::LEFT);
                    break;
                case SDLK_z:
                    _state.reset(Button::B);
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
                    _state.set(Button::UP);
                    break;
                case SDLK_RETURN:
                    _state.set(Button::SELECT);
                    break;

                case SDLK_DOWN:
                    _state.set(Button::DOWN);
                    break;
                case SDLK_SPACE:
                    _state.set(Button::START);
                    break;

                case SDLK_RIGHT:
                    _state.set(Button::RIGHT);
                    break;
                case SDLK_a:
                    _state.set(Button::A);
                    break;

                case SDLK_LEFT:
                    _state.set(Button::LEFT);
                    break;
                case SDLK_z:
                    _state.set(Button::B);
                    break;
                }
            }
        }
    }

    void Gamepad::setState(uint8_t value)
    {
        _buttonSelected = ((value & 0x20) == 0x20);
    }

    uint8_t Gamepad::getState()
    {
        return _buttonSelected ? (static_cast<uint8_t>(_state.to_ulong()) >> 4) : (static_cast<uint8_t>(_state.to_ulong()) << 4) >> 4;
    }

    void Gamepad::setChangePalette(const bool &value)
    {
        _changedPalette = value;
    }

    bool Gamepad::getChangePalette()
    {
        return _changedPalette;
    }

    bool Gamepad::isButtonSelected()
    {
        return _buttonSelected;
    }
}