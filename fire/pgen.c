#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>

#include <SDL.h>

struct particle {
	SDL_Point point;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
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
		pp->point.x = startx;
		pp->point.y = starty;
	}
	return p;
}

void particle_update(struct particle* p, int len) {
	for (int i = 0; i < len; i++) {
		struct particle* prt = &p[i];
		prt->point.x += rand() % 2;
		prt->point.y += rand() % 2;
		prt->r = rand() % 255;
		prt->g = rand() % 255;
		prt->b = rand() % 255;
		prt->life--;
		//printf("\t%d, %d\n", prt->point.x, prt->point.y);
		//printf("\t%p, size = %d\n", prt, sizeof(prt));

		if (prt->life <= 0) {
			prt->point.x = 10;
			prt->point.y = 10;
			prt->life = rand() % 1000;
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
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

	int pcount = 500;
	struct particle* particles = particle_create(pcount, 10, 10);

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
			SDL_SetRenderDrawColor(renderer, prt->r, prt->g, prt->b, 255);
			SDL_RenderDrawPoint(renderer, prt->point.x, prt->point.y);
		}

		SDL_RenderPresent(renderer);

		SDL_Delay(10);
	}

	particle_free(particles);


	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
