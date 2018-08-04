#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>

#include <SDL.h>

#define debug(s); fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, s);

int xmouse = 400, ymouse = 300;

int myrand(int min, int max) {
	return rand() % (max + 1 - min) + min;
}

float floatrand(float min, float max) {
	float scale = rand() / (float) RAND_MAX;
	return min + scale * (max - min);
}

struct particle {
	float x;
	float y;
	float dx;
	float dy;
	int w;
	int h;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
	int maxlife;
	int life;
};

// TODO stop initializing this cruft by default on the heap. make an init
// which accepts an array instead.

void particle_init(struct particle* pp, float startx, float starty) {
	memset(pp, 0, sizeof(struct particle));

	pp->r = rand() % 254;
	pp->g = rand() % 255;
	pp->b = rand() % 255;

	float distx = ((float)xmouse - 400.0f) / 400.0f;
	float disty = ((float)ymouse - 300.0f) / 300.0f;
	pp->dx = floatrand(-distx, distx);
	pp->dy = floatrand(-disty, disty);

	pp->x = startx;
	pp->y = starty;

	pp->maxlife = rand() % 1000;
	pp->life = pp->maxlife;

	int size = myrand(1, 2);
	pp->w = size;
	pp->h = size;
}

void particle_update(struct particle* prt) {
	prt->a = (float)prt->life / (float)prt->maxlife * 255;
	prt->life--;
	//printf("\t%d, %d\n", prt->point.x, prt->point.y);
	//printf("\t%p, size = %d\n", prt, sizeof(prt));
	//
	if (prt->y >= 600 - 4) {
		prt->y = 600 - 4;
		prt->dy = 0;
		prt->dx = 0;
	} else {
		prt->w = (float)prt->life / (float)prt->maxlife * 2 + 1;
		prt->h = (float)prt->life / (float)prt->maxlife * 2 + 1;
		prt->x += prt->dx;
		prt->y += prt->dy;
		// gravity:
		prt->dy += 0.009f;
	}

	// When the particle reaches its end of life, reset it
	// back to the original position with reinitialized parameters.
	if (prt->life <= 0) {
		particle_init(prt, 800 / 2, 600 / 2);
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
		"Pixel renderer",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		800, 600,
		SDL_WINDOW_SHOWN);

	if (window == NULL) {
		fprintf(stderr, "Cannot open window: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

	int pcount = 8000;
	struct particle pzz[pcount];
	for (int i = 0; i < pcount; i++) {
		particle_init(&pzz[i], 800 / 2, 600 / 2);
	}

	SDL_Event e;
	bool quit = false;
	bool pause = false;
	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				debug("type is quit?");
				quit = true;
			}

			if(e.type == SDL_MOUSEMOTION) {
				SDL_GetMouseState(&xmouse,&ymouse);
			}

			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_ESCAPE:
					debug("escape done");
					quit = true;
					break;
				case SDLK_SPACE:
					if (e.key.type == SDL_KEYDOWN) {
						pause = !pause;
						printf("pausing: %d\n", pause);
					}
					break;
				}
			}
		}

		if (pause) {
			continue;
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);


		for (int i = 0; i < pcount; i++) {
			particle_update(&pzz[i]);
		}

		for (int i = 0; i < pcount; i++) {
			struct particle* prt = &pzz[i];
			SDL_SetRenderDrawColor(renderer, prt->r, prt->g, prt->b, prt->a);
			SDL_Rect rect = {
				.x = prt->x,
				.y = prt->y,
				.w = prt->w,
				.h = prt->h
			};
			SDL_RenderFillRect(renderer, &rect);
		}

		SDL_RenderPresent(renderer);

		SDL_Delay(5);
	}

	debug("destroying stuff, we're out of the loop");
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
