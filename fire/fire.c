#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include <SDL.h>

inline int color() {
	return rand() % 255;
}

const int width = 10;
const int height = 10;

inline short getval(short* array, int x, int y) {
	return array[y * width + x];
}

inline void setval(short* array, int x, int y, short val) {
	array[y * width + x] = val;
}

void printarr(short* array) {
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			printf("%4d,", getval(array, x, y));
		}
		printf("\n");
	}
	printf("\n");
}

void burn(SDL_Surface* target, short* array) {
	// initialize the bottom row with cruft.
	for (int x = 0; x < width; x++) {
		setval(array, x, height - 1, color());
	}

	printarr(array);

	int tilewidth  = target->w / width;
	int tileheight = target->h / width;

	// calculate the rest
	for (int y = height - 1; y > 1; y--) {
		for (int x = 0; x < width; x++) {
			short bl = getval(array, y - 1, x - 1);
			short br = getval(array, y - 1, x + 1);
			short ib = getval(array, y - 1, x    );
			short bb = getval(array, y - 2, x    );
			setval(array, x, y, (bl + br + ib + bb) / 4.04);
		}
	}

	printarr(array);
	exit(1);
	// draw it!
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			int idx = y * width + x;
			SDL_Rect rect = {x * tilewidth, y * tileheight, tilewidth, tileheight};
			SDL_FillRect(target, &rect, SDL_MapRGB(target->format, array[idx], 0, 0));
		}
	}

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
	memset(ss, 0, w * h * sizeof(short));

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

		burn(surface, ss);

		SDL_Delay(100);
		SDL_UpdateWindowSurface(window);
	}

	free(ss);
	//SDL_FreeSurface(surfPaint);
	SDL_FreeSurface(surface);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
