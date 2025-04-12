#include "utilitiesProvider.h"
#include "gameBoyProvider.h"
#include "registersProvider.h"
#include "gamepad.h"
#include "defs.h"
#ifdef EMSCRIPTEN
#include <SDL2/SDL.h>
#else
#include "SDL.h"
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
#ifndef EMSCRIPTEN
            if (provider::UtilitiesProvider::getInstance()->debugMode)
            {
                ImGui_ImplSDL2_ProcessEvent(&event);
            }
#endif

            if (event.type == SDL_QUIT)
            {
                exit(ExitState::MANUAL_STOP);
            }
            else if (event.type == SDL_DROPFILE)
            {
                // Resetting gambeoy instance and loading new ROM
                char *droppedFile = event.drop.file;
                if (droppedFile)
                {
                    std::string filePath(droppedFile);
                    gasyboy::provider::UtilitiesProvider::getInstance()->newRomFilePath = filePath;
                    gasyboy::provider::UtilitiesProvider::getInstance()->wasReset = true;
                    SDL_free(droppedFile);
                }
            }

            else if (event.type == SDL_KEYDOWN)
            {
                provider::RegistersProvider::getInstance()->setStopMode(false);

                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    exit(ExitState::MANUAL_STOP);
                    break;

                case SDLK_DOLLAR:
                    gasyboy::provider::UtilitiesProvider::getInstance()->wasRefreshed = true;
                    break;

                case SDLK_KP_MULTIPLY:
                case SDLK_ASTERISK:
                    gasyboy::provider::UtilitiesProvider::getInstance()->wasReset = true;
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

        if (provider::UtilitiesProvider::getInstance()->wasReset)
        {
            provider::GameBoyProvider::getInstance()->reset();
            provider::UtilitiesProvider::getInstance()->wasReset = false;
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