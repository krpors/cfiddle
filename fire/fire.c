#include <assert.h>
#include <math.h>
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

void matrix_init(struct matrix* m, uint32_t width, uint32_t height);
void matrix_free(struct matrix* m);
inline uint32_t matrix_get(const struct matrix* m, int x, int y);
inline void matrix_set(struct matrix* m, int x, int y, uint32_t val);
inline void matrix_getrgb(const struct matrix* m, int x, int y, uint8_t* r, uint8_t* g, uint8_t* b);
void matrix_print(const struct matrix* m, FILE* output);
void matrix_update(struct matrix* m);

void matrix_init(struct matrix* m, uint32_t width, uint32_t height) {
	m->width = width;
	m->height = height;
	m->array = malloc(width * height * sizeof(uint32_t));
	memset(m->array, 0, width*height* sizeof(uint32_t));
}

void matrix_free(struct matrix* m) {
	free(m->array);
}

inline uint32_t matrix_get(const struct matrix* m, int x, int y) {
	assert(m != NULL);
	assert(x >= 0);
	assert(x <= m->width);
	assert(y >= 0);
	assert(y <= m->height);

	// we do a modulo m->width to prevent to go out of bounds.
	return m->array[y * m->width + x % m->width];
}

inline void matrix_set(struct matrix* m, int x, int y, uint32_t val) {
	assert(m != NULL);
	assert(x >= 0 && x <= m->width);
	assert(y >= 0 && y <= m->height);

	m->array[y * m->width + x] = val;
}

inline void matrix_setrgb(struct matrix* m, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
	matrix_set(m, x, y, (r << 16) | (g << 8) | (b));
}

inline void matrix_getrgb(const struct matrix* m, int x, int y, uint8_t* r, uint8_t* g, uint8_t* b) {
	uint32_t val = matrix_get(m, x, y);

	// we may provide NULL pointers to omit assigning values.
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
	assert(m != NULL);

	// initialize the bottom row with cruft.
	for (int x = 0; x < m->width; x++) {
		matrix_setrgb(m, x, m->height - 2, color(), 0x00, 0x00);
		uint8_t r, g, b;
		matrix_getrgb(m, x, m->height - 2, &r, &g, &b);
	}

	// calculate the rest
	for (int y = 0; y < m->height - 2; y++) {
		for (int x = 0; x < m->width; x++) {
			uint8_t
				rbl, // red component, below left (x - 1, y + 1)
				rbr, // red component, below right (x + 1, y + 1)
				rib, // red component, immediately below (y - 1)
				rbb; // red component, below-below (y - 2)

			uint8_t exp1, exp2;
			matrix_getrgb(m, x - 1, y + 1, &rbl, NULL, NULL);
			matrix_getrgb(m, x + 1, y + 1, &rbr, NULL, NULL);
			matrix_getrgb(m, x    , y + 1, &rib, NULL, NULL);
			matrix_getrgb(m, x    , y + 2, &rbb, NULL, NULL);
			matrix_getrgb(m, x - 2, y, &exp1, NULL, NULL);
			matrix_getrgb(m, x + 3, y, &exp2, NULL, NULL);

			uint16_t newval = (rbl + rbr + rib + rbb) / 4.0000001;
			matrix_setrgb(m, x, y, newval, 0, 0);
		}
	}
}

void matrix_update2(struct matrix* m) {
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
	int tilewidth  = 4;//target->w / m->width;
	int tileheight = 2;//target->h / m->height;

	// draw it!
	for (int y = 0; y < m->height; y++) {
		for (int x = 0; x < m->width; x++) {
			uint8_t r,g,b;
			matrix_getrgb(m, x, y, &r, &g, &b);

			SDL_Rect rect = {x * tilewidth, y * tileheight, tilewidth, tileheight};
			SDL_FillRect(target, &rect, SDL_MapRGB(target->format, r, g, b));
		}
	}
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
		"Fire!",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		800, 600,
		SDL_WINDOW_SHOWN);

	if (window == NULL) {
		fprintf(stderr, "Cannot open window: %s\n", SDL_GetError());
		exit(1);
	}

	struct matrix m;
	matrix_init(&m, 320, 240);

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

#ifndef NDEBUG
		matrix_print(&m, stdout);
		pause = true;
#endif
		burn(surface, &m);

		SDL_Delay(20);
		SDL_UpdateWindowSurface(window);
	}

	matrix_free(&m);
	//SDL_FreeSurface(surfPaint);
	SDL_FreeSurface(surface);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
