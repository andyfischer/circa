/* 

    TestGfxBlit - test program to check custom RGBA blitter

    Copyright (C) A. Schiffler, December 2007
    
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
#include "SDL/SDL_gfxBlitFunc.h"


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
	/* Set the screen to gray */
	if ( SDL_LockSurface(screen) == 0 ) {
		Uint32 gray1, gray2;
		Uint8 *pixels;
		gray1 = SDL_MapRGB(screen->format, 64, 64, 64);
		gray2 = SDL_MapRGB(screen->format, 32, 32, 32);
		pixels = (Uint8 *)screen->pixels;
		for ( i=0; i<screen->h; ++i ) {
		 if ((i % 32)<16) {
		  memset(pixels, gray1, screen->w*screen->format->BytesPerPixel);
		 } else {
		  memset(pixels, gray2, screen->w*screen->format->BytesPerPixel);
		 }
		 pixels += screen->pitch;
		}
		SDL_UnlockSurface(screen);
	}
}


void Draw(SDL_Surface *screen)
{
 int rate,i,x,y,s;
 SDL_Rect dest,clip;
 SDL_Surface *texture_image;
 SDL_Surface *texture_target1;
 SDL_Surface *texture_target2;
 FPSmanager fpsm;
 Uint32 rmask, gmask, bmask, amask;

 /* Define masks for 32bit surface */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
 rmask = 0xff000000;
 gmask = 0x00ff0000;
 bmask = 0x0000ff00;
 amask = 0x000000ff;
#else 
 rmask = 0x000000ff;
 gmask = 0x0000ff00;
 bmask = 0x00ff0000;
 amask = 0xff000000;                               
#endif
 
 /* Create semi-transparent textured surface */
 s=64;
 texture_image = SDL_DisplayFormatAlpha(SDL_CreateRGBSurface(SDL_SWSURFACE, s, s, 32, rmask, gmask, bmask, amask));
 /* Add some color */
 boxRGBA(texture_image, 0,  0, s/2, s/2, 255, 0, 0, 255); 
 boxRGBA(texture_image, s/2, 0, s, s/2, 0, 255, 0, 255); 
 boxRGBA(texture_image, 0, s/2, s/2, s, 0, 0, 255, 255); 
 boxRGBA(texture_image, s/2, s/2, s, s, 255, 255, 255, 255);  
 /* Make 75%-transparent */
 SDL_gfxSetAlpha(texture_image, 96);
 /* Set alpha channel use to per-pixel blending */
 SDL_SetAlpha(texture_image, SDL_SRCALPHA, 255);
                                
 /* Create an all transparent surface */
 texture_target1 = SDL_DisplayFormatAlpha(SDL_CreateRGBSurface(SDL_SWSURFACE, 256, 256, 32, rmask, gmask, bmask, amask));
 /* Make 75%-transparent */
 SDL_gfxSetAlpha(texture_target1, 64);
 /* Set alpha channel use to per-pixel blending */
 SDL_SetAlpha(texture_target1, SDL_SRCALPHA, 255);

 /* Create an all transparent surface (2) */
 texture_target2 = SDL_DisplayFormatAlpha(SDL_CreateRGBSurface(SDL_SWSURFACE, 256, 256, 32, rmask, gmask, bmask, amask));
 /* Make 75%-transparent */
 SDL_gfxSetAlpha(texture_target2, 64);
 /* Set alpha channel use to per-pixel blending */
 SDL_SetAlpha(texture_target2, SDL_SRCALPHA, 255);

 /* Define clipping region for left box */
 clip.x = 640/2-256-10 ;
 clip.y = 480/2-256/2 ;
 clip.w = 256;
 clip.h = 256;
  
 /* Initialize Framerate manager */  
 SDL_initFramerate(&fpsm);

 /* Set/switch framerate */
 rate=15;
 SDL_setFramerate(&fpsm,rate);

 
 /* --- Drawing loop */
 while (1) {
  
  /* Event handler */
  HandleEvent();
  
  /* Black screen */
  ClearScreen(screen);

  /* Random position of new texture */
  x=(rand() % (256+2*s))-s;
  y=(rand() % (256+2*s))-s;

  /* Same for comparison texture */
  dest.x = x;
  dest.y = y;
  dest.w = texture_image->w;
  dest.h = texture_image->h;
  SDL_BlitSurface(texture_image, NULL, texture_target1, &dest);

  /* Blit image into the target using custom Blit function. */
  dest.x = x;
  dest.y = y;
  dest.w = texture_image->w;
  dest.h = texture_image->h;
  SDL_gfxBlitRGBA(texture_image, NULL, texture_target2, &dest);
  
  /* Draw comparison target on screen (left) */
  dest.x = 640/2-256-10;
  dest.y = 480/2-256/2;
  dest.w = 256;
  dest.h = 256;
  SDL_BlitSurface(texture_target1, NULL, screen, &dest);

  /* Draw combiner target on screen (right) */
  dest.x = 640/2+10;
  dest.y = 480/2-256/2;
  dest.w = 256;
  dest.h = 256;
  SDL_BlitSurface(texture_target2, NULL, screen, &dest);

  /* Draw some frames with titles */
  rectangleRGBA(screen, 640/2-256-10-1, 480/2-256/2-1, 640/2-256-10-1+257, 480/2-256/2-1+257,  255, 255, 255, 255);
  rectangleRGBA(screen, 640/2+10-1, 480/2-256/2-1, 640/2+10-1+257, 480/2-256/2-1+257, 255, 255, 255, 255);
  stringRGBA (screen, 640/2-256-10-1, 480/2-256/2-1-24, "Image   --sdlBlit--> Target1", 255, 255, 255, 255);
  stringRGBA (screen, 640/2-256-10-1, 480/2-256/2-1-12, "Target1 --sdlBlit--> Screen", 255, 255, 255, 255);
  stringRGBA (screen, 640/2+10-1, 480/2-256/2-1-24, "Image   --gfxBlit--> Target2", 255, 255, 255, 255);  
  stringRGBA (screen, 640/2+10-1, 480/2-256/2-1-12, "Target2 --sdlBlit--> Screen", 255, 255, 255, 255);  

  stringRGBA (screen, 640/2-256-10-1, 480/2-256/2-1-60, "gfxBlitRGBA Demo: target A=64 (25%), image A=96 (37%)", 255, 255, 255, 255);  
  
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
	fprintf (stderr,"gfxBlitRGBA test\n");

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
	SDL_WM_SetCaption("gfxBlitRGBA", "gfxBlitRGBA");

	/* Do all the drawing work */
	Draw (screen);
	
	return(0);
}
