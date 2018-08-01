#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>

#include <SDL.h>

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
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
	int maxlife;
	int life;
};

// TODO stop initializing this cruft by default on the heap. make an init
// which accepts an array instead.
struct particle* particle_create(int howmuch, int startx, int starty) {
	struct particle* p = malloc(howmuch * sizeof(struct particle));
	for (int i = 0; i < howmuch; i++) {
		struct particle* pp = &p[i];
		memset(pp, 0, sizeof(struct particle));
		pp->life = 0;
		pp->maxlife = 0;
		pp->dx = floatrand(-1.0f, 1.0f);
		pp->dy = floatrand(0.0f, 1.0f);
		pp->x = startx;
		pp->y = starty;
	}
	return p;
}

void particle_update(struct particle* p, int len) {
	for (int i = 0; i < len; i++) {
		struct particle* prt = &p[i];
		prt->x += prt->dx;
		prt->y += prt->dy;
		// gravity:
		prt->dy += 0.009f;
		prt->a = (float)prt->life / (float)prt->maxlife * 255;
		prt->life--;
		//printf("\t%d, %d\n", prt->point.x, prt->point.y);
		//printf("\t%p, size = %d\n", prt, sizeof(prt));

		if (prt->life <= 0) {
			prt->r = rand() % 255;
			prt->g = rand() % 255;
			prt->b = rand() % 255;
			prt->dx = floatrand(-1.0f, 1.0f);
			prt->dy = floatrand(0.0f, -1.5f);
			prt->x = 800/2;
			prt->y = 600/2/2;
			prt->maxlife = rand() % 1000;
			prt->life = prt->maxlife;
		}
	}
}

void particle_free(struct particle* p) {
	free(p);
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

	int pcount = 1000;
	struct particle* particles = particle_create(pcount, 800/2, 600/2);

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

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		particle_update(particles, pcount);

		for (int i = 0; i < pcount; i++) {
			struct particle* prt = &particles[i];
			SDL_SetRenderDrawColor(renderer, prt->r, prt->g, prt->b, prt->a);
			SDL_RenderDrawPoint(renderer, floor(prt->x - 0.5f), floor(prt->y - 0.5f));
			SDL_RenderDrawPoint(renderer, floor(prt->x - 0.5f), floor(prt->y) + 0.5f);
			SDL_RenderDrawPoint(renderer, floor(prt->x + 0.5f), floor(prt->y) + 0.5f);
			SDL_RenderDrawPoint(renderer, floor(prt->x + 0.5f), floor(prt->y) - 0.5f);
		}

		SDL_RenderPresent(renderer);

		SDL_Delay(5);
	}

	particle_free(particles);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
