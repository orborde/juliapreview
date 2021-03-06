/*
  juliapreview
  A tool to show the Julia set of a given Mandelbrot

  USAGE:
  Can be called with no arguments
  juliapreview LEFTX TOPY RIGHTX BOTTOMY will set the Mandelbrot's region
  to the given coordinates
*/

#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "complex.h"

typedef struct
{
  complex topleft;
  complex bottomright;
}
complex_region;

/* Screen parameters */
#define DEFAULT_HEIGHT (350)
#define DEFAULT_WIDTH (DEFAULT_HEIGHT * 2)
int HEIGHT = DEFAULT_HEIGHT;
int WIDTH = DEFAULT_WIDTH;

SDL_Surface * screen;
SDL_Rect mandelbrot_screen;
SDL_Rect julia_screen;

/* Rendering parameters */
const Uint32 MAXITERS = 255;
complex_region mandelbrot_region =
  {
    {-2, 1.5},
    {1, -1.5}
  };
complex_region julia_region =
  {
    {-2,2},
    {2,-2}
  };

/* Visualization parameters */
int RGB_PERIOD = 10;

/* Function prototypes */
int configure_video(int width, int height);
int inbounds(int x, int y)
{return (x < WIDTH && y < HEIGHT && x >= 0 && y >= 0);}

void putPixel(SDL_Surface * screen, int x, int y, Uint32 color);

Uint32 visualize_rgb(Uint32 iters, Uint32 period, Uint32 maxiters);

/* Draws the mandelbrot onscreen using the given colormap */
void draw_mandelbrot(SDL_Surface * screen,
                     complex_region region, SDL_Rect screen_region,
                     Uint32 * colormap,
                     unsigned maxiters);
unsigned mandelbrot_iterate(complex c, unsigned maxiters);
/* Draws the Julia onscreen using the given colormap */
void draw_julia(SDL_Surface * screen,
                complex_region region, SDL_Rect screen_region,
                Uint32 * colormap,
                unsigned maxiters,
                complex c);
unsigned julia_iterate(complex z, const complex c, double escape,
                       unsigned maxiters);

double linmap(double x1, double y1, double x2, double y2, double xt);

/* Main Function */
int main (int argc, char * argv[])
{
  /* Fetch commandline arguments */
  if (argc >= 5)
    {
      double leftx = atof(argv[1]);
      double topy = atof(argv[2]);
      double rightx = atof(argv[3]);
      double bottomy = atof(argv[4]);

      complex_region temp = { {leftx, topy}, {rightx, bottomy} };
      mandelbrot_region = temp;

      fprintf(stderr,"Mandelbrot region set to (%lf,%lf) - (%lf,%lf)\n",
              leftx, topy, rightx, bottomy);
    }

  /* SDL initialization stuff */
  // init video, check for success
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    { fprintf(stderr,"Video init failed\n"); return -1; };

  // reg SDL_Quit to be called at exit to clean up
  atexit(SDL_Quit);

  fprintf(stderr,"SDL is up and running!\n");

  if (configure_video(DEFAULT_WIDTH, DEFAULT_HEIGHT))
    { fprintf(stderr,"Video modeset failed\n"); return -1; };
  fprintf(stderr,"Video is set!\n");

  /*** Our initialization stuff ***/
  fprintf(stderr,"Now setting up fractal things\n");
  // Fill out the colormap array
  Uint32 colormap[MAXITERS];
  {
    unsigned i;
    for (i=0; i<=MAXITERS; i++)
      {
        colormap[i] =
          visualize_rgb(i, RGB_PERIOD, MAXITERS);
      }
  }
  // Draw the mandelbrot
  {
    fprintf(stderr,"Rendering mandelbrot...\n");
    Uint32 start = SDL_GetTicks();
    draw_mandelbrot(screen, mandelbrot_region, mandelbrot_screen,
                    colormap, MAXITERS);
    Uint32 stop = SDL_GetTicks();
    fprintf(stderr,"  Mandelbrot took %lums\n", (long unsigned) stop - start);
  }
  // Assign a c and render an initial Julia
  complex c = {.233, .53780};
  fprintf(stderr,"Rendering initial Julia\n");
  draw_julia(screen, julia_region, julia_screen, colormap, MAXITERS, c);

  fprintf(stderr,"Entering main loop\n");
  // main loop!
  while(1)
    {
      // Wait for an event
      SDL_WaitEvent(NULL);

      // handle events
      SDL_Event event;
      while(SDL_PollEvent(&event))
        {
          switch(event.type)
            {
            case SDL_VIDEORESIZE:
              // Reconfigure video
              if (configure_video(event.resize.w, event.resize.h))
                {
                  fprintf(stderr,"Error on video reconfigure! Quitting...\n");
                  return -1;
                }
              // Redraw the Mandelbrot and current Julia in their new regions
              draw_mandelbrot(screen, mandelbrot_region, mandelbrot_screen,
                              colormap, MAXITERS);
              draw_julia(screen, julia_region, julia_screen,
                         colormap, MAXITERS, c);
              break;
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
      // Read mouse state and rerender Julia if needed
      {
        int x,y;
        static int zoom_selecting = 0;
        static complex zoom_upperleft = {0, 0};
        Uint8 state = SDL_GetMouseState(&x, &y);
        if (x >= mandelbrot_screen.x &&
            y >= mandelbrot_screen.y &&
            x < mandelbrot_screen.x + mandelbrot_screen.w &&
            y < mandelbrot_screen.y + mandelbrot_screen.h)
          {
            // If left mouse button pressed, redraw a Julia
            if (state & SDL_BUTTON(1))
              {
                c.r = linmap(mandelbrot_screen.x,
                             mandelbrot_region.topleft.r,
                             mandelbrot_screen.x+mandelbrot_screen.w,
                             mandelbrot_region.bottomright.r,
                             x);
                c.i = linmap(mandelbrot_screen.y,
                             mandelbrot_region.topleft.i,
                             mandelbrot_screen.y+mandelbrot_screen.h,
                             mandelbrot_region.bottomright.i,
                             y);
                draw_julia(screen, julia_region, julia_screen,
                           colormap, MAXITERS, c);
              } // Left mouse button action

          } // In the mandelbrot region?
        else
          {
            zoom_selecting = 0;
          }
      } // Mouse state reading block
    } // main loop
  return 0;
}

double linmap(double x1, double y1, double x2, double y2, double xt)
{
  return y1 + (y2 - y1)*(xt - x1)/(x2 - x1);
}

int configure_video(int width, int height)
{
  /*  // Reset global screen dimensions according to what changed
      if (width != WIDTH)
      {
      HEIGHT = width/2;
      WIDTH = HEIGHT * 2;
      }
      else if (height != HEIGHT)
      {
      HEIGHT = height;
      WIDTH = HEIGHT * 2;
      }*/
  /// Ah...whatever. Just reassign the #$%*&#$^ variables!
  HEIGHT = height;
  WIDTH = width;

  // Reset screen portion variables
  julia_screen.x = 0;
  julia_screen.y = 0;
  julia_screen.w = WIDTH/2;
  julia_screen.h = HEIGHT;

  mandelbrot_screen.x = WIDTH/2;
  mandelbrot_screen.y = 0;
  mandelbrot_screen.w = WIDTH - WIDTH/2;
  mandelbrot_screen.h = HEIGHT;

  screen = SDL_SetVideoMode(WIDTH,HEIGHT,32,SDL_SWSURFACE | SDL_RESIZABLE);
  if (screen == NULL) { fprintf(stderr,"Video modeset failed\n"); return -1; };

  return 0;
};

void putPixel(SDL_Surface * screen, int x, int y, Uint32 color)
{
  //  fprintf(stderr," %x@%d,%d", color, x, y);
  unsigned int offset =
    x + (screen->pitch >> 2) * y;
  ((Uint32 *) screen->pixels)[offset] = color;
}

Uint32 visualize_rgb(Uint32 iters, Uint32 period, Uint32 maxiters)
{
  // Non-escaping values are black
  if (iters >= maxiters)
    return SDL_MapRGB(screen->format, 0, 0, 0);

  iters %= (period * 3);
  Uint32 value =
    (iters % period) * 255 / period;
  if (iters >= (period * 2))
    return SDL_MapRGB(screen->format, value, 0, 0);
  else if (iters >= period)
    return SDL_MapRGB(screen->format, 0, value, 0);
  else
    return SDL_MapRGB(screen->format, 0, 0, value);
}

void draw_mandelbrot(SDL_Surface * screen,
                     complex_region region, SDL_Rect screen_region,
                     Uint32 * colormap,
                     unsigned maxiters)
{
  // lock teh surface
  if (SDL_MUSTLOCK(screen))
    if (SDL_LockSurface(screen) < 0) return;

  int i,j;
  for (j=0; j<screen_region.h; j++)
    {
      complex c = {0,0};
      c.i = region.topleft.i +
        (region.bottomright.i - region.topleft.i) * j / screen_region.h;

      for (i=0; i<screen_region.w; i++)
        {
          c.r =
            region.topleft.r +
            (region.bottomright.r - region.topleft.r) * i / screen_region.w;
          unsigned iters = mandelbrot_iterate(c, maxiters);
          //      fprintf(stderr,"%lf,%lf=%u(%lu) ", c.r, c.i, iters,colormap[iters]);
          Uint32 color =
            colormap[iters];
          putPixel(screen, screen_region.x + i, screen_region.y + j, color);
        }
      //      putchar('\n');
    }

  // unlock teh surface
  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);

  // update!
  /*SDL_UpdateRect(screen,
    screen_region.x, screen_region.y,
    screen_region.w, screen_region.h);*/
  //  SDL_UpdateRect(screen, 0,0,0,0);
  SDL_UpdateRects(screen, 1, &screen_region);


}

unsigned iterate(complex z, complex c, unsigned maxiters)
{
  unsigned iters = 0;

  while (++iters < maxiters && complex_sqmag(z) <= 4)
    z = complex_add(complex_mult(z, z), c);

  return iters;
}

unsigned mandelbrot_iterate(complex c, unsigned maxiters)
{
  complex z = {0,0};
  return iterate(z,c, maxiters);
}


void draw_julia(SDL_Surface * screen,
                complex_region region, SDL_Rect screen_region,
                Uint32 * colormap,
                unsigned maxiters,
                complex c)
{
  printf("c=(%lf,%lf)\n", c.r, c.i);
  // lock teh surface
  if (SDL_MUSTLOCK(screen))
    if (SDL_LockSurface(screen) < 0) return;

  int i,j;
  for (j=0; j<screen_region.h; j++)
    {
      complex z = {0,0};
      z.i = region.topleft.i +
        (region.bottomright.i - region.topleft.i) * j / screen_region.h;

      for (i=0; i<screen_region.w; i++)
        {
          z.r =
            region.topleft.r +
            (region.bottomright.r - region.topleft.r) * i / screen_region.w;
          unsigned iters = julia_iterate(z,c,2, maxiters);
          //      fprintf(stderr,"%lf,%lf=%u(%lu) ", c.r, c.i, iters,colormap[iters]);
          Uint32 color =
            colormap[iters];
          putPixel(screen, screen_region.x + i, screen_region.y + j, color);
        }
      //      putchar('\n');
    }

  // unlock teh surface
  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);

  // update!
  /*  SDL_UpdateRect(screen,
      screen_region.x, screen_region.y,
      screen_region.w, screen_region.h);*/
  //  SDL_UpdateRect(screen, 0,0,0,0);
  SDL_UpdateRects(screen, 1, &screen_region);


}

unsigned julia_iterate(complex z, const complex c, double escape,
                       unsigned maxiters)
{
  return iterate(z,c, maxiters);
}
