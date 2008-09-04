/* 

    TestGfxTexture - test program for textured polygon routine

    Copyright (C) A. Schiffler, December 2006
    (Contributed by Kees Jongenburger)
    
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
 int i,rate,x,y,dx,dy;
 int psize = 150; 
 float sin_start = 0;
 float sin_amp = 100;
 Sint16 polygon_x[psize], polygon_y[psize];
 Sint16 polygon_alpha_x[4], polygon_alpha_y[4];
 SDL_Surface *texture;
 SDL_Surface *texture_alpha;
 FPSmanager fpsm;
  
 /* Load texture surfaces */
 texture = SDL_LoadBMP("texture.bmp");
 texture_alpha = SDL_LoadBMP("texture_alpha.bmp");
 SDL_SetAlpha(texture_alpha, SDL_SRCALPHA, 128);

 /* Initialize variables */
 srand(time(NULL));
 i=0;
 x=screen->w/2;
 y=screen->h/2;
 dx=7;
 dy=5;

 /* Initialize Framerate manager */  
 SDL_initFramerate(&fpsm);

 /* Polygon for blended texture */
 polygon_alpha_x[0]= 0;
 polygon_alpha_y[0]= 0;
 polygon_alpha_x[1]= screen->w/2 ;
 polygon_alpha_y[1]= 0;
 polygon_alpha_x[2]= screen->w*2 /3;
 polygon_alpha_y[2]= screen->h;
 polygon_alpha_x[3]= 0;
 polygon_alpha_y[3]= screen->h;

 /* Set/switch framerate */
 rate=25;
 SDL_setFramerate(&fpsm,rate);
 
 /* Drawing loop */
 while (1) {
  
  /* Generate wave polygon */
  sin_start++;
  polygon_x[0]= 0;
  polygon_y[0]= screen->h;
  polygon_x[1]= 0;
  polygon_y[1]= screen->h/2;    
  for (i=2; i < psize -2 ; i++){
      polygon_x[i]= (screen->w  * (i-2)) / (psize -5) ;
      polygon_y[i]= sin(sin_start/100) * 200 + screen->h /2 -sin( (i +sin_start) / 20) * sin_amp;
  }
  
  polygon_x[psize-2]= screen->w;
  polygon_y[psize-2]= screen->h/2;
  polygon_x[psize-1]= screen->w;
  polygon_y[psize-1]= screen->h;

  /* Event handler */
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
  texturedPolygon(screen,polygon_x,polygon_y,psize,texture,-(screen->w  * (sin_start-2)) / (psize -5), -sin(sin_start/100) * 200);
  texturedPolygon(screen,polygon_alpha_x,polygon_alpha_y,4,texture_alpha,sin_start,-sin_start);

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
	fprintf (stderr,"texturedPolygon test\n");

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
	SDL_WM_SetCaption("texturedPolygon test", "texturedPolygon");

	/* Do all the drawing work */
	Draw (screen);
	
	return(0);
}
