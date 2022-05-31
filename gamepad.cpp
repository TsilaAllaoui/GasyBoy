#include "gamepad.h"

Gamepad::Gamepad()
{
	buttonSelected = true;
	directionSelected = true;
	currState = 0;

	for (int i=0; i<8; i++)
		keys[i] = false;
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
				keys[A] = true;
				break;
			case SDLK_z:
				keys[B] = true;
				break;
			case SDLK_RETURN:
				keys[SELECT] = true;
				break;
			case SDLK_SPACE:
				keys[START] = true;
				break;
			case SDLK_UP:
				keys[UP] = true;
				break;
			case SDLK_DOWN:
				keys[DOWN] = true;
				break;
			case SDLK_LEFT:
				keys[LEFT] = true;
				break;
			case SDLK_RIGHT:
				keys[RIGHT] = true;
				break;
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
				keys[A] = false;
				break;
			case SDLK_z:
				keys[B] = false;
				break;
			case SDLK_RETURN:
				keys[SELECT] = false;
				break;
			case SDLK_SPACE:
				keys[START] = false;
				break;
			case SDLK_UP:
				keys[UP] = false;
				break;
			case SDLK_DOWN:
				keys[DOWN] = false;
				break;
			case SDLK_LEFT:
				keys[LEFT] = false;
				break;
			case SDLK_RIGHT:
				keys[RIGHT] = false;
				break;
			}
		}
	}
}

void Gamepad::setState(uint8_t value)
{
	buttonSelected = ((value & 0x20) == 0x20);
	directionSelected = ((value & 0x10) == 0x10);
}

uint8_t Gamepad::getState()
{
	currState = 0xCF;
	if (!buttonSelected)
	{
		if (keys[A]) currState &= ~(1 << 0);
		if (keys[B]) currState &= ~(1 << 1);
		if (keys[SELECT]) currState &= ~(1 << 2);
		if (keys[START]) currState &= ~(1 << 3);
		currState |= 0x10;
	}
	else if (!directionSelected)
	{
		if (keys[RIGHT]) currState &= ~(1 << 0);
		if (keys[LEFT]) currState &= ~(1 << 1);
		if (keys[UP]) currState &= ~(1 << 2);
		if (keys[DOWN]) currState &= ~(1 << 3);
		currState |= 0x20;
	}
	return currState;
}