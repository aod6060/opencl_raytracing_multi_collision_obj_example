#include "sys.h"



namespace input {

	InputState keys[Keyboard::KB_SIZE];

	void init() {
		for (int i = 0; i < Keyboard::KB_SIZE; i++) {
			keys[i] = InputState::IS_RELEASE;
		}
	}

	void pollEvents(SDL_Event& e) {
		// Keydown
		if (e.type == SDL_KEYDOWN) {
			Keyboard key = (Keyboard)e.key.keysym.scancode;

			if (keys[key] == InputState::IS_RELEASE) {
				keys[key] = InputState::IS_DOWN;
			}
		}
		// Keyup
		if (e.type == SDL_KEYUP) {
			Keyboard key = (Keyboard)e.key.keysym.scancode;

			if (keys[key] == InputState::IS_PRESSED) {
				keys[key] = InputState::IS_UP;
			}
		}
	}

	void update() {
		for (int i = 0; i < Keyboard::KB_SIZE; i++) {
			if (keys[i] == InputState::IS_DOWN) {
				keys[i] = InputState::IS_PRESSED;
			}

			if (keys[i] == InputState::IS_UP) {
				keys[i] = InputState::IS_RELEASE;
			}
		}
	}

	bool isKeyRelease(const Keyboard& key) {
		return keys[key] == InputState::IS_RELEASE;
	}

	bool isKeyDown(const Keyboard& key) {
		return keys[key] == InputState::IS_DOWN;
	}

	bool isKeyPressed(const Keyboard& key) {
		return keys[key] == InputState::IS_PRESSED;
	}

	bool isKeyUp(const Keyboard& key) {
		return keys[key] == InputState::IS_UP;
	}
}