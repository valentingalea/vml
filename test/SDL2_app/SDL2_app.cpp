#include "../c4droid.h"

#include "SDL2_app.h"
#include <cassert>

SDL2_app::SDL2_app()
{
	IsAlive = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0;
	if (!IsAlive) {
		log();
		return;
	}

	Window.reset(
		SDL_CreateWindow(
			"vml",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			640, 480,
			0
		),
		SDL_DestroyWindow
	);
	if (!Window.get()) {
		log();
		return;
	}

	Renderer.reset(
		SDL_CreateRenderer(
			Window.get(), -1, SDL_RENDERER_ACCELERATED
		),
		SDL_DestroyRenderer
	);
	if (!Renderer.get()) {
		log();
		return;
	}
}

SDL2_app::~SDL2_app()
{
	SDL_Quit();
}

void SDL2_app::log()
{
	auto err = SDL_GetError();
	if (err && err[0]) {
		SDL_Log("%s\n", err);
		IsAlive = false;
	}
}

void SDL2_app::run()
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

		SDL_SetRenderDrawColor(Renderer.get(), 255, 0, 0, 255);
		SDL_RenderClear(Renderer.get());
		SDL_RenderPresent(Renderer.get());
		SDL_Delay(1);
	}
}

int main()
{
	SDL2_app app;
	app.run();

	return 0;
}