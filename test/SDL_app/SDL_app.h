#pragma once

#include <SDL.h>
#undef main

#include <memory>
#include <vector>

class SDL_app
{
public:
	SDL_app();
	~SDL_app();
	void run();
	void draw();

	struct WorkDef
	{
		int height_start;
		int height_end;
	};
private:
	bool IsAlive = false;
	
	SDL_Surface *Screen = nullptr;
	std::shared_ptr<SDL_Surface> OffScreen;
	std::vector<WorkDef> WorkItems;
	
	void log();
};
