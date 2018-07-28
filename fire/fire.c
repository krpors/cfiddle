#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>

#include <SDL.h>

inline int color() {
	return rand() % 255;
}

/*
 * A pixel matrix, where each element in the array represents
 * a uint32_t RGB value.
 */
struct matrix {
	uint32_t* array;
	int width;
	int height;
};

void matrix_init(struct matrix* m, int width, int height) {
	m->width = width;
	m->height = height;
	m->array = malloc(width * height * sizeof(uint32_t));
	memset(m->array, 0, width*height* sizeof(uint32_t));
}

void matrix_free(struct matrix* m) {
	free(m->array);
}

inline uint32_t matrix_get(const struct matrix* m, int x, int y) {
	return m->array[y * m->width + x % m->width];
}

inline void matrix_set(struct matrix* m, int x, int y, uint32_t val) {
	m->array[y * m->width + x] = val;
}

inline void matrix_setrgb(struct matrix* m, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
	matrix_set(m, x, y, (r << 16) | (g << 8) | (b));
}

inline void matrix_getrgb(const struct matrix* m, int x, int y, uint8_t* r, uint8_t* g, uint8_t* b) {
	uint32_t val = matrix_get(m, x, y);
	if (r != NULL) {
		*r = (uint8_t)((val & 0xFF0000) >> 16);
	}
	if (g != NULL) {
		*g = (uint8_t)((val & 0x00FF00) >>  8);
	}
	if (b != NULL) {
		*b = (uint8_t)((val & 0x0000FF));
	}
}

void matrix_update(struct matrix* m) {
	// initialize the bottom row with cruft.
	for (int x = 0; x < m->width; x++) {
		matrix_setrgb(m, x, m->height - 2, color(), 0x00, 0x00);
		uint8_t r, g, b;
		matrix_getrgb(m, x, m->height - 2, &r, &g, &b);
	}

	// calculate the rest
	for (int y = 0; y < m->height - 2; y++) {
		for (int x = 0; x < m->width; x++) {
			uint8_t rbl, rbr, rib, rbb;
			matrix_getrgb(m, x - 1, y + 1, &rbl, NULL, NULL);
			matrix_getrgb(m, x + 1, y + 1, &rbr, NULL, NULL);
			matrix_getrgb(m, x    , y + 1, &rib, NULL, NULL);
			matrix_getrgb(m, x    , y + 2, &rbb, NULL, NULL);
			short newval = (rbl + rbr + rib + rbb) / 4.04;
			matrix_setrgb(m, x, y, newval, 0, 1);
		}
	}
}

/*
 * Print out the matrix to the given output file (can also be
 * stdout, stderr).
 */
void matrix_print(const struct matrix* m, FILE* output) {
	fprintf(output, "w: %4d, h: %4d\n", m->width, m->height);
	for (int y = 0; y < m->height; y++) {
		for (int x = 0; x < m->width; x++) {
			fprintf(output, "%6x,", matrix_get(m, x, y));
		}
		fprintf(output, "\n");
	}
	fprintf(output, "\n");
}

void burn(SDL_Surface* target, const struct matrix* m) {
	int tilewidth  = target->w / m->width;
	int tileheight = target->h / m->height;

	// draw it!
	for (int y = 0; y < m->height - 3; y++) {
		for (int x = 0; x < m->width; x++) {
			uint8_t r,g,b;
			matrix_getrgb(m, x, y, &r, &g, &b);

			SDL_Rect rect = {x * tilewidth, y * tileheight, tilewidth, tileheight};
			SDL_FillRect(target, &rect, SDL_MapRGB(target->format, 0, r, 0));
		}
	}
}

int _main(int argc, char* argv[]) {
	(void)(argc);
	(void)(argv);

	srand(time(NULL));

	struct matrix m;
	matrix_init(&m, 5, 5);
	matrix_print(&m, stdout);
	matrix_update(&m);
	matrix_print(&m, stdout);
	matrix_free(&m);

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

	struct matrix m;
	matrix_init(&m, 200, 100);

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

		matrix_update(&m);
		burn(surface, &m);

		SDL_Delay(50);
		SDL_UpdateWindowSurface(window);
	}

	matrix_free(&m);
	//SDL_FreeSurface(surfPaint);
	SDL_FreeSurface(surface);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
