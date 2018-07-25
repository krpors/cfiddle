#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include <SDL.h>

inline int color() {
	return rand() % 255;
}

const int width = 100;
const int height = 80;

inline short getval(short* array, int x, int y) {
	return array[y * width + x % width];
}

inline void setval(short* array, int x, int y, short val) {
	array[y * width + x] = val;
}

void printarr(short* array) {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			printf("%4d,", getval(array, x, y));
		}
		printf("\n");
	}
	printf("\n");
}

void update(short* array) {
	// initialize the bottom row with cruft.
	for (int x = 0; x < width; x++) {
		setval(array, x, height - 2, color());
	}

	//printf("INIT 2nd ROW:\n");
	//printarr(array);

	// calculate the rest
	for (int y = 0; y < height - 2; y++) {
		for (int x = 0; x < width; x++) {
			short bl = getval(array, x - 1, y + 1);
			short br = getval(array, x + 1, y + 1);
			short ib = getval(array, x    , y + 1);
			short bb = getval(array, x    , y + 2);
			short newval = (bl + br + ib + bb) / 4.04;
#if 0
			printf("now altering %d, %d (has value %d)\n", x, y, getval(array, x, y));
			printf("\t%d %d %d %d\n", bl, br, ib, bb);
			printf("\tnewval: %d\n", newval);
#endif
			setval(array, x, y, newval);
		}
	}

	//printf("UPDATE ALL:\n");
	//printarr(array);
}

void burn(SDL_Surface* target, short* array) {
	update(array);

	int tilewidth  = target->w / width;
	int tileheight = target->h / height;

	// draw it!
	for (int y = 0; y < height - 3; y++) {
		for (int x = 0; x < width; x++) {
			short val = getval(array, x, y);
			SDL_Rect rect = {x * tilewidth, y * tileheight, tilewidth, tileheight};
			SDL_FillRect(target, &rect, SDL_MapRGB(target->format, 0, val, 0));
		}
	}
}

int _main(int argc, char* argv[]) {
	(void)(argc);
	(void)(argv);

	srand(time(NULL));

	short* ss = malloc(width * height * sizeof(short));
	memset(ss, 0, width * height * sizeof(short));

	update(ss);
	update(ss);
	update(ss);

	free(ss);
	return 0;
}

int main(int argc, char* argv[]) {
	(void)(argc);
	(void)(argv);

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

	short* ss = malloc(width * height * sizeof(short));
	memset(ss, 0, width * height * sizeof(short));

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

		SDL_Delay(50);
		SDL_UpdateWindowSurface(window);
	}

	free(ss);
	//SDL_FreeSurface(surfPaint);
	SDL_FreeSurface(surface);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
