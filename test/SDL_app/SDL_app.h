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
	void draw();

private:
	bool IsAlive = false;
	SDL_Surface *Screen = nullptr;
	std::shared_ptr<SDL_Surface> OffScreen;
	
	void log();
};
