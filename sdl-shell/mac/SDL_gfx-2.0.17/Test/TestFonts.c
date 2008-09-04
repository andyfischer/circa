/* 

    TestFonts
    
    Test dynamic font loading code

    Copyright (C) A. Schiffler, August 2001, GPL

*/

#ifdef WIN32
 #include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "SDL.h"

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



#define NUM_SDLGFX_FONTS	7

void Draw(SDL_Surface *screen)
{
 int i;
 FILE *file;
 char *myfont;
 char *fontfile[NUM_SDLGFX_FONTS] = {
 	"default",
 	"5x7.fnt",
 	"7x13.fnt",
 	"7x13B.fnt",
 	"7x13O.fnt",
 	"9x18.fnt",
 	"9x18B.fnt",
 	};
 int  fontw[NUM_SDLGFX_FONTS] = {
        8,
 	5,
	7,
	7,
	7,
	9,
	9,
	};	
 int  fonth[NUM_SDLGFX_FONTS] = {
        8,
 	7,
	13,
	13,
	13,
	18,
	18,
	};	
 int  fontsize[NUM_SDLGFX_FONTS] = {
        1,
 	1792,
	3328,
	3328,
	3328,
	9216,
	9216,
	};	
 char mytext[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
 int x,y;
 char filename[128];
    
 /* Black screen */
 ClearScreen(screen);

 /* Render all fonts */ 
 y=0; 
 for (i=0; i<NUM_SDLGFX_FONTS; i++) {
  fprintf(stderr,"Drawing Font: size %ix%i, file '%s', %i bytes\n",fontw[i], fonth[i], fontfile[i],fontsize[i]);
  /* Reset line pos */
  x=4;
  /* Allocate memory for font data */
  myfont=(char *)malloc(fontsize[i]);
  if (myfont) {
   if (strcmp(fontfile[i],"default")) {
    /* Load a font data */
    sprintf(filename,"../Fonts/%s",fontfile[i]);
    file = fopen(filename,"r");
    fread(myfont,fontsize[i],1,file);
    fclose(file);
    /* Set font data and use it */
    gfxPrimitivesSetFont(myfont,fontw[i],fonth[i]);
   }   
   y += fonth[i];
   stringRGBA(screen,x,y,fontfile[i],255,255,255,255);
   x += 128;
   stringRGBA(screen,x,y,mytext,255,255,255,255);
   y += 10;
   /* Clean up font-data */
   free(myfont);
  }
 }
  
 /* Display by flipping screens */
 SDL_Flip(screen);
 
 /* Wait */
 while(1) {
  HandleEvent();
  SDL_Delay(100);
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
	fprintf (stderr,"Font Test\n");

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
	SDL_WM_SetCaption("TestFonts", "testfonts");

	/* Do all the drawing work */
	Draw (screen);
	
	return(0);
}
