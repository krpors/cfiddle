#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include <SDL.h>

inline int color() {
	return rand() % 255;
}

int main(int argc, char* argv[]) {
	srand(time(NULL));

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Cannot init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Window* window = SDL_CreateWindow(
		"SDL Tutorial",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		800, 600,
		SDL_WINDOW_SHOWN);

	if (window == NULL) {
		fprintf(stderr, "Cannot open window: %s\n", SDL_GetError());
		exit(1);
	}

	const int w = 64;
	const int h = 64;
	short* ss = malloc(w * h * sizeof(short));
	for (int i = 0; i < (w*h); i++) {
		ss[i] = color();
	}

	//SDL_Surface* surfPaint = SDL_CreateRGBSurface(0, 200, 200, 32, 0,0,0,0);
	SDL_Surface* surface = SDL_GetWindowSurface(window);

	SDL_Event e;
	bool quit = false;
	bool pause = false;
	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}

			switch (e.key.keysym.sym) {
			case SDLK_ESCAPE: quit = true;    break;
			case SDLK_SPACE:
				if (e.key.type == SDL_KEYDOWN) {
				pause = !pause;
				printf("pausing: %d\n", pause);
				}
				break;
			}
		}

		if (pause) {
			continue;
		}

		//SDL_FillRect(surfPaint, NULL, SDL_MapRGB(surfPaint->format, 255, 0, 0));
		//SDL_BlitSurface(surfPaint, NULL, surface, NULL);
		for (int i = 0; i < (w*h); i++) {
			ss[i] = color();
		}
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				int idx = y * w + x;
				SDL_Rect rect = {x*5,y*5,5,5};
				SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, ss[idx], 0, 0));
			}
		}
		SDL_UpdateWindowSurface(window);
	}

	free(ss);
	//SDL_FreeSurface(surfPaint);
	SDL_FreeSurface(surface);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
