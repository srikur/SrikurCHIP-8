#include "includes.h"
#include "chip8.h"

using namespace std;

constexpr int screen_width = 64;
constexpr int screen_height = 32;

u16 opcode;
u8 keys[16];
u32* pixels;

CHIP8* cpu;

int main(int argc, char** argv) {

	SDL_Window* window = NULL;
	SDL_Event event;
	bool quit = false;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Failed to initialize SDL! Error: %s\n", SDL_GetError());
	}
	else {
		window = SDL_CreateWindow("Srikur's CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			printf("Failed to create the SDL window! Error: %s\n", SDL_GetError());
		}
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(renderer, screen_width, screen_height);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, screen_width, screen_height);
	pixels = (u32*)malloc(screen_height * screen_width * sizeof(u32));

	cpu = new CHIP8();
	cpu->loadGame("ROMNAME");

	while (!quit) {
		SDL_UpdateTexture(texture, NULL, pixels, screen_width * sizeof(u32));
		SDL_WaitEvent(&event);

		cpu->emulateCycle();

		switch (event.type) {
		case SDL_QUIT:
			quit = true;
			break;
		case SDL_KEYDOWN:
			break;
		}

		if (cpu->drawFlag) {
			/* Redraw */

			for (int i = 0; i < 64 * 32; i++) {
				pixels[i] = (0x00FFFFFF * cpu->graphics[i]) | 0xFF000000;
			}

			SDL_UpdateTexture(texture, NULL, pixels, screen_width * sizeof(u32));
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}

		cpu->setKeys();

		this_thread::sleep_for(chrono::microseconds(1500));
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	free(pixels);
	delete cpu;

	return 0;
}