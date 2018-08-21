#include "SDL_app.h"
#include <cassert>
#include <cstdio>

SDL_app::SDL_app()
{
	IsAlive = SDL_Init(SDL_INIT_VIDEO) == 0;
	if (!IsAlive) {
		log();
		return;
	}

	SDL_WM_SetCaption("sdl_app", "app");
	Screen = SDL_SetVideoMode(100, 100, 24, 0);
	if (!Screen) {
		log();
		return;
	}
}

SDL_app::~SDL_app()
{
	SDL_Quit();
}

void SDL_app::log()
{
	auto err = SDL_GetError();
	if (err && err[0]) {
		printf("%s\n", err);
		IsAlive = false;
	}
}

void SDL_app::run()
{
	assert(IsAlive);
	if (!IsAlive) {
		return;
	}

	SDL_Event event;
	bool running = true;

	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					running = false;
				}
			}
		}

		SDL_FillRect(Screen, NULL, 255);
		SDL_Flip(Screen);
		SDL_Delay(1);
	}
}

int main()
{
	SDL_app app;
	app.run();

	return 0;
}