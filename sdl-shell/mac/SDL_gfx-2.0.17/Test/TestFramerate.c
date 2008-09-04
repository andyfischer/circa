/* 

    SDL_framerate - test program for framerate manager 

    Copyright (C) A. Schiffler, August 2001

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifdef WIN32
 #include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "SDL.h"

#include "SDL/SDL_framerate.h"
#include "SDL/SDL_gfxPrimitives.h"

void HandleEvent()
{
	SDL_Event event; 

	/* Check for events */
        while ( SDL_PollEvent(&event) ) {
                        switch (event.type) {
			 case SDL_KEYDOWN:
			 case SDL_QUIT:
                                        exit(0);
                                        break;
			}
	}
}

void ClearScreen(SDL_Surface *screen)
{
	int i;
	/* Set the screen to black */
	if ( SDL_LockSurface(screen) == 0 ) {
		Uint32 black;
		Uint8 *pixels;
		black = SDL_MapRGB(screen->format, 0, 0, 0);
		pixels = (Uint8 *)screen->pixels;
		for ( i=0; i<screen->h; ++i ) {
			memset(pixels, black,
				screen->w*screen->format->BytesPerPixel);
			pixels += screen->pitch;
		}
		SDL_UnlockSurface(screen);
	}
}

void Draw(SDL_Surface *screen)
{
 int i,rate,x,y,dx,dy,r,g,b;
 FPSmanager fpsm;
 
 /* Initialize variables */
 srand(time(NULL));
 i=0;
 x=screen->w/2;
 y=screen->h/2;
 dx=7;
 dy=5;
 r=g=b=255;
  
 SDL_initFramerate(&fpsm);

 while (1) {
  
  /* Set/switch framerate */
  i -= 1;
  if (i<0) {
   /* Set new rate */
   rate=5+5*(rand() % 10);
   SDL_setFramerate(&fpsm,rate);
   printf ("\nFramerate set to %i Hz ...\n\n",rate);
   /* New timeout */
   i=2*rate;
   /* New Color */
   r=rand() & 255;
   g=rand() & 255;
   b=rand() & 255;
  }

  HandleEvent();
  
  /* Black screen */
  ClearScreen(screen);

  /* Move */
  x += dx;
  y += dy;
  
  /* Reflect */
  if ((x<0) || (x>screen->w)) { dx=-dx; }
  if ((y<0) || (y>screen->h)) { dy=-dy; }

  /* Draw */
  filledCircleRGBA (screen,x,y,30,r,g,b,255);
  circleRGBA(screen,x,y,30,255,255,255,255);
    
  /* Display by flipping screens */
  SDL_Flip(screen);

  /* Delay to fix rate */                   
  SDL_framerateDelay(&fpsm);  
 }
}

#ifdef WIN32
 extern char ** __argv;
 extern int __argc;
 int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
#else // non WIN32
 int main ( int argc, char *argv[] )
#endif
{
	SDL_Surface *screen;
	int w, h;
	int desired_bpp;
	Uint32 video_flags;
#ifdef WIN32
	int argc;
	char **argv;

	argv = __argv;
	argc = __argc;
#endif
	/* Title */
	fprintf (stderr,"SDL_framerate test\n");

	/* Set default options and check command-line */
	w = 640;
	h = 480;
	desired_bpp = 0;
	video_flags = 0;
	while ( argc > 1 ) {
		if ( strcmp(argv[1], "-width") == 0 ) {
			if ( argv[2] && ((w = atoi(argv[2])) > 0) ) {
				argv += 2;
				argc -= 2;
			} else {
				fprintf(stderr,
				"The -width option requires an argument\n");
				exit(1);
			}
		} else
		if ( strcmp(argv[1], "-height") == 0 ) {
			if ( argv[2] && ((h = atoi(argv[2])) > 0) ) {
				argv += 2;
				argc -= 2;
			} else {
				fprintf(stderr,
				"The -height option requires an argument\n");
				exit(1);
			}
		} else
		if ( strcmp(argv[1], "-bpp") == 0 ) {
			if ( argv[2] ) {
				desired_bpp = atoi(argv[2]);
				argv += 2;
				argc -= 2;
			} else {
				fprintf(stderr,
				"The -bpp option requires an argument\n");
				exit(1);
			}
		} else
		if ( strcmp(argv[1], "-warp") == 0 ) {
			video_flags |= SDL_HWPALETTE;
			argv += 1;
			argc -= 1;
		} else
		if ( strcmp(argv[1], "-hw") == 0 ) {
			video_flags |= SDL_HWSURFACE;
			argv += 1;
			argc -= 1;
		} else
		if ( strcmp(argv[1], "-fullscreen") == 0 ) {
			video_flags |= SDL_FULLSCREEN;
			argv += 1;
			argc -= 1;
		} else
			break;
	}

	/* Force double buffering */
	video_flags |= SDL_DOUBLEBUF;

	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr,
			"Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);			/* Clean up on exit */

	/* Initialize the display */
	screen = SDL_SetVideoMode(w, h, desired_bpp, video_flags);
	if ( screen == NULL ) {
		fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n",
					w, h, desired_bpp, SDL_GetError());
		exit(1);
	}

	/* Show some info */
	printf("Set %dx%dx%d mode\n",
			screen->w, screen->h, screen->format->BitsPerPixel);
	printf("Video surface located in %s memory.\n",
			(screen->flags&SDL_HWSURFACE) ? "video" : "system");
	
	/* Check for double buffering */
	if ( screen->flags & SDL_DOUBLEBUF ) {
		printf("Double-buffering enabled - good!\n");
	}

	/* Set the window manager title bar */
	SDL_WM_SetCaption("SDL_framerate test", "framerate");

	/* Do all the drawing work */
	Draw (screen);
	
	return(0);
}
