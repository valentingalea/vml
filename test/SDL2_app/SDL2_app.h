#pragma once

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <memory>

class SDL2_app
{
public:
	SDL2_app();
	~SDL2_app();
	void run();
	void draw();

private:
	bool IsAlive = false;
	std::shared_ptr<SDL_Window> Window;
	std::shared_ptr<SDL_Renderer> Renderer;
	std::shared_ptr<SDL_Texture> Texture;
	
	void log();
};