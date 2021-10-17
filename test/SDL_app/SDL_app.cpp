#include "../c4droid.h"

// config #define's
// SCR_W8
// SCR_H8
// DUMP_FPS
// APP_??? (see sandbox.h)
#include "../shader/sandbox.h"

 vec3 sandbox::iResolution	= vec3(SCR_W8, SCR_H8, 0); // viewport resolution (in pixels)
float sandbox::iTime		= 0; // shader playback time (in seconds)
float sandbox::iTimeDelta	= 0; // render time (in seconds)
 vec4 sandbox::iMouse;			 // mouse pixel coords. xy: current (if MLB down), zw: click
 vec4 sandbox::iDate;			 // (year, month, day, time in seconds)

#include <SDL.h>
#undef main

#include <cassert>
#include <cstdio>
#include <memory>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

 class SDL_app
 {
 public:
	 SDL_app();
	 ~SDL_app();
	 void run();
	 void draw();

 private:
	 bool IsAlive = false;

	 SDL_Surface* Screen = nullptr;
	 std::shared_ptr<SDL_Surface> OffScreen;

	 std::vector<std::thread> WorkerThreads;

	 void log();
 };


SDL_app::SDL_app()
{
	IsAlive = SDL_Init(SDL_INIT_VIDEO) == 0;
	if (!IsAlive) {
		log();
		return;
	}

	constexpr int bit_depth = 24;
	int mask_r = 0x000000ff;
	int mask_g = 0x0000ff00;
	int mask_b = 0x00ff0000;

	SDL_WM_SetCaption("sdl_app", "app");
	Screen = SDL_SetVideoMode(SCR_W8, SCR_H8, bit_depth, 0);
	if (!Screen) {
		log();
		return;
	}

	OffScreen.reset(
		SDL_CreateRGBSurface(SDL_SWSURFACE, SCR_W8, SCR_H8, bit_depth, mask_r, mask_g, mask_b, 0),
		SDL_FreeSurface
	);
	if (!OffScreen.get()) {
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

std::atomic_int WorkerCounter = -1;
std::atomic_bool StartFrame = false;
std::atomic_bool StopWork = false;

struct WorkDef
{
	int height_start;
	int height_end;
};
std::vector<WorkDef> WorkerDefs;

void ThreadWorkerFunc(const WorkDef work, const SDL_Surface* bmp)
{
	bool sliceDone = false;
	while (true) {
		if (StartFrame && sliceDone) {
			sliceDone = false;
			continue;
		}
		if (WorkerCounter < 0 || !StartFrame || sliceDone) {
			std::this_thread::yield();
			continue;
		}
		if (StopWork) {
			return;
		}

		sandbox::fragment_shader shader;
		for (int y = work.height_start; y < work.height_end; ++y) {
			uint8_t* ptr = reinterpret_cast<uint8_t*>(bmp->pixels) + y * bmp->pitch;
			for (int x = 0; x < bmp->w; ++x) {
				shader.gl_FragCoord = vec2(static_cast<float>(x), bmp->h - 1.0f - y);
				shader.main(shader.gl_FragColor, shader.gl_FragCoord);
				const auto color = sandbox::clamp(shader.gl_FragColor, 0.0f, 1.0f);

				*ptr++ = static_cast<uint8_t>(255 * color.r + 0.5f);
				*ptr++ = static_cast<uint8_t>(255 * color.g + 0.5f);
				*ptr++ = static_cast<uint8_t>(255 * color.b + 0.5f);
			}
		}

		WorkerCounter++;
		sliceDone = true;

		if (work.height_start == 0) {
			return; // special case for main thread who needs to do other stuff
		}
	}
}

void SDL_app::run()
{
	assert(IsAlive);
	if (!IsAlive) {
		return;
	}

	const int count = std::thread::hardware_concurrency();
	const int slice = SCR_H8 / count;
	for (int i = 0; i < count; i++) {
		WorkerDefs.push_back(WorkDef{i * slice, i * slice + slice});
	}
	for (int i = 1; i < count; i++) {
		WorkerThreads.push_back(std::thread(
			ThreadWorkerFunc,
			WorkerDefs[i],
			OffScreen.get()
		));
		WorkerThreads.back().detach();
	}

	auto event = SDL_Event();
	auto running = true;
	auto time_start = std::chrono::system_clock::now();

	constexpr auto fmt = "curr: %3.2f; max: %3.2f; avrg: %3.2f;\r";
	auto avrg_fps = 0.f;
	auto max_fps = 0.f;
	auto frame_num = 1;
#ifdef DUMP_FPS
	auto file_closer = [](FILE* f) { fclose(f); };
	std::unique_ptr<FILE, decltype(file_closer)> file = { fopen("fps.txt", "wt+"), file_closer };
#endif

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
		SDL_Flip(Screen);

		auto time_now = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsed_seconds = time_now - time_start;
		std::chrono::duration<float> frame_seconds = time_now - time_frame;
		sandbox::iTime = elapsed_seconds.count();
		sandbox::iTimeDelta = frame_seconds.count();

		auto curr_fps = 1.f / sandbox::iTimeDelta;
		max_fps = std::max(max_fps, curr_fps);
		avrg_fps += (curr_fps - avrg_fps) / frame_num++; // https://en.wikipedia.org/wiki/Moving_average
		printf(fmt, curr_fps, max_fps, avrg_fps);
	}

#ifdef DUMP_FPS
	fprintf(file.get(), fmt, 0.f, max_fps, avrg_fps);
#endif

	StopWork = false; // signal to kill the worker threads
}

void SDL_app::draw()
{
	WorkerCounter = 0; // (re)start all threads
	StartFrame = true;

	// do the first slice on this thread
	ThreadWorkerFunc(WorkerDefs[0], OffScreen.get());

	// wait for other threads to do their slices
	while (WorkerCounter < WorkerThreads.size()) {
		std::this_thread::yield();
	}

	StartFrame = false;
	SDL_BlitSurface(OffScreen.get(), NULL, Screen, NULL);
}

int main()
{
	SDL_app app;
	app.run();

	return 0;
}