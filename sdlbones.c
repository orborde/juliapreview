#include <SDL/SDL.h>
#include <stdio.h>
#include "complex.h"

typedef struct
{
  complex topleft;
  complex bottomright;
}
complex_region;

/* Screen parameters */
#define WIDTH 500
#define HEIGHT 250

SDL_Surface * screen;
SDL_Rect mandelbrot_screen =
  {
    WIDTH/2 + 1, 0,
    WIDTH/2, HEIGHT
  };
SDL_Rect julia_screen =
  {
    WIDTH/2, 0,
    WIDTH/2, HEIGHT
  };

/* Rendering parameters */
const Uint32 MAXITERS = 255;
const complex_region mandelbrot_region =
  {
    {-2, 1.5},
    {1, -1.5}
  };

/* Visualization parameters */
int RGB_PERIOD = 10;
Uint32 colormap[MAXITERS];

int inbounds(int x, int y)
{
	return (x < WIDTH && y < HEIGHT && x >= 0 && y >= 0);
}

void putPixel(int x, int y, Uint32 color)
{
//	printf(" %x@%d,%d", color, x, y);
	unsigned int offset =
		x + (screen->pitch >> 2) * y;
	((Uint32 *) screen->pixels)[offset] = color;
}

void render(Uint32 time)
{
	// lock teh surface
	if (SDL_MUSTLOCK(screen))
		if (SDL_LockSurface(screen) < 0) return;

	// rendering stuff here
	SDL_FillRect(screen, NULL, time);

	// unlock teh surface
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	// update!
	SDL_UpdateRect(screen, 0, 0, WIDTH, HEIGHT);
}

int main ()
{
	// init video, check for success
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{ printf("Video init failed\n"); return 1; };

	// reg SDL_Quit to be called at exit to clean up
	atexit(SDL_Quit);

	printf("SDL is up and running!\n");

	// make me a screen worthy of Mordor
	screen = SDL_SetVideoMode(WIDTH,HEIGHT,32,SDL_SWSURFACE);
	if (screen == NULL) { printf("Video modeset failed\n"); return 1; };
	printf("Video is set!\n");


	// initialize the basic colors
	Uint32 white = SDL_MapRGB(screen->format, 255, 255, 255);
	Uint32 black = SDL_MapRGB(screen->format, 0, 0, 0);

	printf("Entering main loop\n");
	Uint32 last_time = SDL_GetTicks();
	// main loop!
	while(1)
	{
		// main render
		Uint32 curtime = SDL_GetTicks();
		render(curtime);

		// handle events
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							return 0;
							break;
					}
					break;
				case SDL_QUIT:
					return 0;
					break;
			}
		} // poll

	} // main loop
	return 0;
}
