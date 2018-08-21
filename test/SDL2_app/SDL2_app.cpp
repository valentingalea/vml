#include "../c4droid.h"

#include "../shader/sandbox.h"

static constexpr int SCR_W8 = 240;
static constexpr int SCR_H8 = 240;

vec3 sandbox::iResolution = vec3(SCR_W8, SCR_H8, 0); // viewport resolution (in pixels)
float sandbox::iTime = 0; // shader playback time (in seconds)
float sandbox::iTimeDelta = 0; // render time (in seconds)

#include "SDL2_app.h"
#include <cassert>
#include <chrono>

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
			SCR_W8, SCR_H8,
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

	Texture.reset(
		SDL_CreateTexture(
			Renderer.get(),
			SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
			SCR_W8, SCR_H8
		),
		SDL_DestroyTexture
	);
	if (!Texture.get()) {
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
	auto time_start = std::chrono::system_clock::now();

	while (running) {
		auto time_frame = std::chrono::system_clock::now();

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

		draw();
		
		auto time_now = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsed_seconds = time_now - time_start;
		std::chrono::duration<float> frame_seconds = time_now - time_frame;
		sandbox::iTime = elapsed_seconds.count();
		sandbox::iTimeDelta = frame_seconds.count();

	//	printf("%2.1f\r", 1.f / sandbox::iTimeDelta);
	}
}

void SDL2_app::draw()
{
	sandbox::fragment_shader shader;

	void *tex_ptr = nullptr;
	int tex_pitch = 0;
	SDL_LockTexture(Texture.get(), nullptr, &tex_ptr, &tex_pitch);

	for (int y = 0; y < SCR_H8; ++y) {
		uint8_t * ptr = reinterpret_cast<uint8_t*>(tex_ptr) + y * tex_pitch;
		for (int x = 0; x < SCR_W8; ++x) {
			shader.gl_FragCoord = vec2(static_cast<float>(x), SCR_H8 - 1.0f - y);
			shader.mainImage(shader.gl_FragColor, shader.gl_FragCoord);
			const auto color = shader.gl_FragColor;

			*ptr++ = static_cast<uint8_t>(255 * color.r + 0.5f);
			*ptr++ = static_cast<uint8_t>(255 * color.g + 0.5f);
			*ptr++ = static_cast<uint8_t>(255 * color.b + 0.5f);
			ptr++;
		}
	}

	SDL_UnlockTexture(Texture.get());
	SDL_RenderCopy(Renderer.get(), Texture.get(), NULL, NULL);
	SDL_RenderPresent(Renderer.get());
}

int main()
{
	SDL2_app app;
	app.run();

	return 0;
}