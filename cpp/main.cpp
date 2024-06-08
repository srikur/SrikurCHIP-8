#include "includes.h"
#include "cpu.h"

using namespace std;

u16 opcode;
u8 keys[16];

u8 keycodes[16] = {
	SDLK_x,
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_q,
	SDLK_w,
	SDLK_e,
	SDLK_a,
	SDLK_s,
	SDLK_d,
	SDLK_z,
	SDLK_c,
	SDLK_4,
	SDLK_r,
	SDLK_f,
	SDLK_v,
};

CHIP8* cpu;

int main(int argc, char** argv) {

	if (argc != 2) {
		printf("Usage: %s <ROM>\n", argv[0]);
		return 1;
	}

	u32 pixels[screen_width * screen_height];

	SDL_Window* window = NULL;
	SDL_Event event;
	bool quit = false;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Failed to initialize SDL! Error: %s\n", SDL_GetError());
	}
	else {
		window = SDL_CreateWindow("Srikur's CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width * 10, screen_height * 15, SDL_WINDOW_SHOWN);
		SDL_SetWindowResizable(window, SDL_TRUE);
		if (window == NULL) {
			printf("Failed to create the SDL window! Error: %s\n", SDL_GetError());
		}
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(renderer, screen_width, screen_height);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);

	cpu = new CHIP8();

	const char* romName = argv[1];
	bool loadResult = cpu->loadGame(romName);
	if (!strcmp(romName, "../roms/INVADERS")) {
		cpu->shift_quirk = 1;
	}
	if (!loadResult) {
		printf("Unable to start the emulation!\n");
		SDL_DestroyTexture(texture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		return 0;
	}

	while (!quit) {

		cpu->emulateCycle();

		if (cpu->drawFlag) {
			/* Redraw */
			cpu->drawFlag = false;

			for (int i = 0; i < screen_width * screen_height; ++i) {
				pixels[i] = (cpu->graphics[i] ? 0xFFFFFFFF : 0xFF000000);
				//pixels[i] = ((0x00FFFFFF * cpu->graphics[i]) | 0xFF000000);
			}

			SDL_UpdateTexture(texture, NULL, pixels, screen_width * sizeof(u32));
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}

		while (SDL_PollEvent(&event)) {

			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					quit = true;
				}
				for (int i = 0; i < 16; i++) {
					if (event.key.keysym.sym == keycodes[i]) {
						cpu->keys[i] = 1;
					}
				}
				break;
			case SDL_KEYUP:
				for (int i = 0; i < 16; i++) {
					if (event.key.keysym.sym == keycodes[i]) {
						cpu->keys[i] = 0;
					}
				}
				break;
			}
		}
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	delete cpu;

	return 0;
}