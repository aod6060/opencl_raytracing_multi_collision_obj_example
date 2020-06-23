#include "sys.h"

namespace app {

	AppConfig* g_appConfig = nullptr;
	bool g_isRunning = true;
	SDL_Window* g_window = nullptr;

	// Timing
	uint32_t pre = 0;
	uint32_t curr = 0;
	float delta = 0.0f;

	// Event
	SDL_Event g_event;

	void init(AppConfig* config) {
		g_appConfig = config;

		SDL_Init(SDL_INIT_EVERYTHING);

		g_window = SDL_CreateWindow(
			g_appConfig->caption.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			g_appConfig->width,
			g_appConfig->height,
			SDL_WINDOW_SHOWN
		);

		input::init();

		if (g_appConfig->initCB) {
			g_appConfig->initCB();
		}
	}

	void update() {
		pre = SDL_GetTicks();

		while (g_isRunning) {
			// Calculate Frame Time Delta
			curr = SDL_GetTicks();
			delta = (curr - pre) / 1000.0f;
			pre = curr;

			// SDL Event
			while (SDL_PollEvent(&g_event)) {
				if (g_event.type == SDL_QUIT) {
					app::exit();
				}

				input::pollEvents(g_event);
			}


			if (g_appConfig->updateCB) {
				g_appConfig->updateCB(delta);
			}

			if (g_appConfig->renderCB) {
				g_appConfig->renderCB();
			}

			input::update();
			SDL_UpdateWindowSurface(g_window);
		}
	}

	void release() {

		if (g_appConfig->releaseCB) {
			g_appConfig->releaseCB();
		}
		
		g_appConfig = nullptr;
		SDL_DestroyWindow(g_window);
		SDL_Quit();
	}

	uint32_t getWidth() {
		return g_appConfig->width;
	}

	uint32_t getHeight() {
		return g_appConfig->height;
	}

	void exit() {
		g_isRunning = false;
	}

	SDL_Surface* getScreenSurface() {
		return SDL_GetWindowSurface(g_window);
	}
}