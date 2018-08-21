#pragma once

#include <SDL.h>
#undef main

#include <memory>

class SDL_app
{
public:
	SDL_app();
	~SDL_app();
	void run();

private:
	bool IsAlive = false;
	SDL_Surface* Screen;
	
	void log();
};
