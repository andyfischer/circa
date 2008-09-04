/* 

    TestRotozoom 
    
    Test program for rotozoom routines

    Copyright (C) A. Schiffler, July 2001, GPL

*/

#ifdef WIN32
 #include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "SDL.h"

#include "SDL/SDL_rotozoom.h"

/* Custom rotation setup */
float custom_angle=0.0;
float custom_fx=1.0;
float custom_fy=1.0;

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
		Uint8 *pixels;
		pixels = (Uint8 *)screen->pixels;
		for ( i=0; i<screen->h; ++i ) {
			memset(pixels, 0,
				screen->w*screen->format->BytesPerPixel);
			pixels += screen->pitch;
		}
		SDL_UnlockSurface(screen);
	}
}

void RotatePicture (SDL_Surface *screen, SDL_Surface *picture, int rotate, int flip, int smooth) 
{
	SDL_Surface *rotozoom_picture;
	SDL_Rect dest;
	int framecount, framemax, frameinc;
	float angle, zoomf, zoomfx, zoomfy;

	/* Rotate and display the picture */
	framemax=4*360; frameinc=1;
	for (framecount=360; framecount<framemax; framecount += frameinc) {
		if ((framecount % 360)==0) frameinc++;
		HandleEvent();
		ClearScreen(screen);
                zoomf=(float)framecount/(float)framemax;
                zoomf=1.5*zoomf*zoomf;
                /* Are we in flipping mode? */
		if (flip) {
		 /* Flip X factor */
                 if (flip & 1) {
                  zoomfx=-zoomf;
                 } else {
                  zoomfx=zoomf;
                 }
                 /* Flip Y factor */
                 if (flip & 2) {
                  zoomfy=-zoomf;
                 } else {
                  zoomfy=zoomf;
                 }
                 angle=framecount*rotate;
                 if ((framecount % 120)==0) {
                  printf ("  Frame: %i   Rotate: angle=%.2f  Zoom: x=%.2f y=%.2f\n",framecount,angle,zoomfx,zoomfy);
                 }
		 if ((rotozoom_picture=rotozoomSurfaceXY (picture, angle, zoomfx, zoomfy, smooth))!=NULL) {
			dest.x = (screen->w - rotozoom_picture->w)/2;;
			dest.y = (screen->h - rotozoom_picture->h)/2;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;
			if ( SDL_BlitSurface(rotozoom_picture, NULL, screen, &dest) < 0 ) {
				fprintf(stderr, "Blit failed: %s\n", SDL_GetError());
				break;
			}
			SDL_FreeSurface(rotozoom_picture);
		 }
		} else {
                 angle=framecount*rotate;
                 if ((framecount % 120)==0) {
                  printf ("  Frame: %i   Rotate: angle=%.2f  Zoom: f=%.2f \n",framecount,angle,zoomf);
                 }
		 if ((rotozoom_picture=rotozoomSurface (picture, angle, zoomf, smooth))!=NULL) {
			dest.x = (screen->w - rotozoom_picture->w)/2;;
			dest.y = (screen->h - rotozoom_picture->h)/2;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;
			if ( SDL_BlitSurface(rotozoom_picture, NULL, screen, &dest) < 0 ) {
				fprintf(stderr, "Blit failed: %s\n", SDL_GetError());
				break;
			}
			SDL_FreeSurface(rotozoom_picture);
		 }
                }
		/* Display by flipping screens */
		SDL_Flip(screen);
	}
	
	if (rotate) {
		/* Final display with angle=0 */
		HandleEvent();
		ClearScreen(screen);
		if (flip) {
 		 if ((rotozoom_picture=rotozoomSurfaceXY (picture, 0.01, zoomfx, zoomfy, smooth))!=NULL) {
			dest.x = (screen->w - rotozoom_picture->w)/2;;
			dest.y = (screen->h - rotozoom_picture->h)/2;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;
			if ( SDL_BlitSurface(rotozoom_picture, NULL, screen, &dest) < 0 ) {
				fprintf(stderr, "Blit failed: %s\n", SDL_GetError());
				return;
			}
			SDL_FreeSurface(rotozoom_picture);
		 }		
		} else {
 		 if ((rotozoom_picture=rotozoomSurface (picture, 0.01, zoomf, smooth))!=NULL) {
			dest.x = (screen->w - rotozoom_picture->w)/2;;
			dest.y = (screen->h - rotozoom_picture->h)/2;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;
			if ( SDL_BlitSurface(rotozoom_picture, NULL, screen, &dest) < 0 ) {
				fprintf(stderr, "Blit failed: %s\n", SDL_GetError());
				return;
			}
			SDL_FreeSurface(rotozoom_picture);
		 }		
	        }
		/* Display by flipping screens */
		SDL_Flip(screen);
	}

	/* Pause for a sec */
	SDL_Delay(1000);
}

void ZoomPicture (SDL_Surface *screen, SDL_Surface *picture, int smooth) 
{
	SDL_Surface *rotozoom_picture;
	SDL_Rect dest;
	int framecount, framemax, frameinc;
	float zoomxf,zoomyf;

	/* Zoom and display the picture */
	framemax=4*360; frameinc=1;
	for (framecount=360; framecount<framemax; framecount += frameinc) {
		if ((framecount % 360)==0) frameinc++;
		HandleEvent();
		ClearScreen(screen);
                zoomxf=(float)framecount/(float)framemax;
                zoomxf=1.5*zoomxf*zoomxf;
                zoomyf=0.5+fabs(1.0*sin((double)framecount/80.0));
                if ((framecount % 120)==0) {
                 printf ("  Frame: %i   Zoom: x=%.2f y=%.2f\n",framecount,zoomxf,zoomyf);
                }
		if ((rotozoom_picture=zoomSurface (picture, zoomxf, zoomyf, smooth))!=NULL) {
			dest.x = (screen->w - rotozoom_picture->w)/2;;
			dest.y = (screen->h - rotozoom_picture->h)/2;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;
			if ( SDL_BlitSurface(rotozoom_picture, NULL, screen, &dest) < 0 ) {
				fprintf(stderr, "Blit failed: %s\n", SDL_GetError());
				break;
			}
			SDL_FreeSurface(rotozoom_picture);
		}

		/* Display by flipping screens */
		SDL_Flip(screen);
	}
	
	/* Pause for a sec */
	SDL_Delay(1000);
}

#define ROTATE_OFF	0
#define ROTATE_ON	1

#define FLIP_OFF	0
#define FLIP_X		1
#define FLIP_Y		2
#define FLIP_XY		3


void CustomTest(SDL_Surface *screen, SDL_Surface *picture, float a, float x, float y){
	SDL_Surface *rotozoom_picture;
	SDL_Rect dest;

	printf ("  Frame: C   Rotate: angle=%.2f  Zoom: fx=%.2f fy=%.2f \n",a,x,y);

	HandleEvent();
	ClearScreen(screen);
 	if ((rotozoom_picture=rotozoomSurfaceXY (picture, a, x, y, 0))!=NULL) {
			dest.x = (screen->w - rotozoom_picture->w)/2;;
			dest.y = (screen->h - rotozoom_picture->h)/2;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;
			if ( SDL_BlitSurface(rotozoom_picture, NULL, screen, &dest) < 0 ) {
				fprintf(stderr, "Blit failed: %s\n", SDL_GetError());
				return;
			}
			SDL_FreeSurface(rotozoom_picture);
	}

	/* Display by flipping screens */
	SDL_Flip(screen);

	SDL_Delay(3000);		
}

void Draw (SDL_Surface *screen, int start)
{
	SDL_Surface *picture, *picture_again;
	char *bmpfile;

	/* --------- 8 bit test -------- */

	if (start<=6) {
	
  	 /* Message */
         fprintf (stderr,"Loading 8bit image\n");

	 /* Load the image into a surface */
	 bmpfile = "sample8.bmp";
	 fprintf(stderr, "Loading picture: %s\n", bmpfile);
	 picture = SDL_LoadBMP(bmpfile);
	 if ( picture == NULL ) {
		fprintf(stderr, "Couldn't load %s: %s\n", bmpfile, SDL_GetError());
		return;
	 }


	 fprintf (stderr,"1.  rotozoom: Rotating and zooming\n");
	 RotatePicture(screen,picture,ROTATE_ON,FLIP_OFF,SMOOTHING_OFF);

	 fprintf (stderr,"2.  rotozoom: Just zooming (angle=0)\n");
	 RotatePicture(screen,picture,ROTATE_OFF,FLIP_OFF,SMOOTHING_OFF);

	 fprintf (stderr,"3.  zoom: Just zooming\n");
	 ZoomPicture(screen,picture,SMOOTHING_OFF);

	 fprintf (stderr,"4.  rotozoom: Rotating and zooming, interpolation on but unused\n");
	 RotatePicture(screen,picture,ROTATE_ON,FLIP_OFF,SMOOTHING_ON);

	 fprintf (stderr,"5.  rotozoom: Just zooming (angle=0), interpolation on but unused\n");
	 RotatePicture(screen,picture,ROTATE_OFF,FLIP_OFF,SMOOTHING_ON);

	 fprintf (stderr,"6.  zoom: Just zooming, interpolation on but unused\n");
	 ZoomPicture(screen,picture,SMOOTHING_ON);

	 /* Free the picture */
	 SDL_FreeSurface(picture);
	
	}

	/* -------- 24 bit test --------- */


	if (start<=12) {

	 /* Message */
         fprintf (stderr,"Loading 24bit image\n");

	 /* Load the image into a surface */
	 bmpfile = "sample24.bmp";
	 fprintf(stderr, "Loading picture: %s\n", bmpfile);
	 picture = SDL_LoadBMP(bmpfile);
	 if ( picture == NULL ) {
		fprintf(stderr, "Couldn't load %s: %s\n", bmpfile, SDL_GetError());
		return;
	 }

	 fprintf (stderr,"7.  rotozoom: Rotating and zooming, no interpolation\n");
	 RotatePicture(screen,picture,ROTATE_ON,FLIP_OFF,SMOOTHING_OFF);

	 fprintf (stderr,"8.  rotozoom: Just zooming (angle=0), no interpolation\n");
	 RotatePicture(screen,picture,ROTATE_ON,FLIP_OFF,SMOOTHING_OFF);

	 fprintf (stderr,"9.  zoom: Just zooming, no interpolation\n");
	 ZoomPicture(screen,picture,SMOOTHING_OFF);


	 fprintf (stderr,"10. rotozoom: Rotating and zooming, with interpolation\n");
	 RotatePicture(screen,picture,ROTATE_ON,FLIP_OFF,SMOOTHING_ON);

	 fprintf (stderr,"11. rotozoom: Just zooming (angle=0), with interpolation\n");
	 RotatePicture(screen,picture,ROTATE_OFF,FLIP_OFF,SMOOTHING_ON);

	 fprintf (stderr,"12. zoom: Just zooming, with interpolation\n");
	 ZoomPicture(screen,picture,SMOOTHING_ON);

	 /* Free the picture */
	 SDL_FreeSurface(picture);

	}
        
	/* -------- 32 bit test --------- */

	if (start<=16) {

	 /* Message */
         fprintf (stderr,"Loading 24bit image\n");

	 /* Load the image into a surface */
	 bmpfile = "sample24.bmp";
	 fprintf(stderr, "Loading picture: %s\n", bmpfile);
	 picture = SDL_LoadBMP(bmpfile);
	 if ( picture == NULL ) {
		fprintf(stderr, "Couldn't load %s: %s\n", bmpfile, SDL_GetError());
		return;
	 }

  	 /* New source surface is 32bit with defined RGBA ordering */
	 /* Much faster to do this once rather than the routine on the fly */
	 fprintf (stderr,"Converting 24bit image into 32bit RGBA surface ...\n");
  	 picture_again = SDL_CreateRGBSurface(SDL_SWSURFACE, picture->w, picture->h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	 SDL_BlitSurface(picture,NULL,picture_again,NULL);
 
	 /* Message */
	 fprintf (stderr,"13. Rotating and zooming, with interpolation\n");
	 RotatePicture(screen,picture_again,ROTATE_ON,FLIP_OFF,SMOOTHING_ON);

	 /* Message */
	 fprintf (stderr,"14. Just zooming (angle=0), with interpolation\n");
	 RotatePicture(screen,picture_again,ROTATE_OFF,FLIP_OFF,SMOOTHING_ON);

	 SDL_FreeSurface(picture_again);

  	 /* New source surface is 32bit with defined ABGR ordering */
	 /* Much faster to do this once rather than the routine on the fly */
	 fprintf (stderr,"Converting 24bit image into 32bit ABGR surface ...\n");
  	 picture_again = SDL_CreateRGBSurface(SDL_SWSURFACE, picture->w, picture->h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	 SDL_BlitSurface(picture,NULL,picture_again,NULL);

	 /* Message */
	 fprintf (stderr,"15. Rotating and zooming, with interpolation\n");
	 RotatePicture(screen,picture_again,ROTATE_ON,FLIP_OFF,SMOOTHING_ON);

	 /* Message */
	 fprintf (stderr,"16. Just zooming (angle=0), with interpolation\n");
	 RotatePicture(screen,picture_again,ROTATE_OFF,FLIP_OFF,SMOOTHING_ON);

	 SDL_FreeSurface(picture_again);

	 /* Free the picture */
	 SDL_FreeSurface(picture);

        }
        
	/* -------- 32 bit flip test --------- */

        if (start<=19) {

	 /* Message */
         fprintf (stderr,"Loading 24bit image\n");
 
	 /* Load the image into a surface */
	 bmpfile = "sample24.bmp";
	 fprintf(stderr, "Loading picture: %s\n", bmpfile);
	 picture = SDL_LoadBMP(bmpfile);
	 if ( picture == NULL ) {
		fprintf(stderr, "Couldn't load %s: %s\n", bmpfile, SDL_GetError());
		return;
	 }
        
  	 /* Excercise flipping functions on 32bit RGBA */
	 fprintf (stderr,"Converting 24bit image into 32bit RGBA surface ...\n");
  	 picture_again = SDL_CreateRGBSurface(SDL_SWSURFACE, picture->w, picture->h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	 SDL_BlitSurface(picture,NULL,picture_again,NULL);

	 /* Message */
	 fprintf (stderr,"17. Rotating with x-flip, no interpolation\n");
	 RotatePicture(screen,picture_again,ROTATE_ON,FLIP_X,SMOOTHING_OFF);

	 /* Message */
	 fprintf (stderr,"18. Rotating with y-flip, no interpolation\n");
	 RotatePicture(screen,picture_again,ROTATE_ON,FLIP_Y,SMOOTHING_OFF);

	 /* Message */
	 fprintf (stderr,"19. Rotating with xy-flip, no interpolation\n");
	 RotatePicture(screen,picture_again,ROTATE_ON,FLIP_XY,SMOOTHING_OFF);

	 SDL_FreeSurface(picture_again);
       
	 /* Free the picture */
	 SDL_FreeSurface(picture);

        }

        if (start<=21) {

 	 /* Message */
         fprintf (stderr,"Loading 24bit image\n");

	 /* Load the image into a surface */
	 bmpfile = "sample24.bmp";
	 fprintf(stderr, "Loading picture: %s\n", bmpfile);
	 picture = SDL_LoadBMP(bmpfile);
	 if ( picture == NULL ) {
		fprintf(stderr, "Couldn't load %s: %s\n", bmpfile, SDL_GetError());
		return;
	 }

   	 /* Excercise flipping functions on 32bit RGBA */
	 fprintf (stderr,"Converting 24bit image into 32bit RGBA surface ...\n");
  	 picture_again = SDL_CreateRGBSurface(SDL_SWSURFACE, picture->w, picture->h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	 SDL_BlitSurface(picture,NULL,picture_again,NULL);

	 fprintf (stderr,"20. CustomTest (32bit)\n");
	 CustomTest(screen, picture_again, custom_angle, custom_fx, custom_fy);

	 SDL_FreeSurface(picture_again);
       
 	 /* Free the picture */
	 SDL_FreeSurface(picture);

 	 /* Message */
         fprintf (stderr,"Loading 8bit image\n");

	 /* Load the image into a surface */
	 bmpfile = "sample8.bmp";
	 fprintf(stderr, "Loading picture: %s\n", bmpfile);
	 picture = SDL_LoadBMP(bmpfile);
	 if ( picture == NULL ) {
		fprintf(stderr, "Couldn't load %s: %s\n", bmpfile, SDL_GetError());
		return;
	 }

	 fprintf (stderr,"21. CustomTest (8bit)\n");
	 CustomTest(screen, picture, custom_angle, custom_fx, custom_fy);

	 /* Free the picture */
	 SDL_FreeSurface(picture);

        }
	
	return;
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
	int start;
#ifdef WIN32
	int argc;
	char **argv;

	argv = __argv;
	argc = __argc;
#endif
	/* Title */
	fprintf (stderr,"SDL_rotozoom test\n");

	/* Set default options and check command-line */
	w = 640;
	h = 480;
	desired_bpp = 0;
	video_flags = 0;
	start = 1;
	while ( argc > 1 ) {
		if ( strcmp(argv[1], "-start") == 0 ) {
			if ( argv[2] && ((start = atoi(argv[2])) > 0) ) {
				argv += 2;
				argc -= 2;
			} else {
				fprintf(stderr,
				"The -start option requires an argument\n");
				exit(1);
			}
		} else
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
		if ( strcmp(argv[1], "-custom") == 0 ) {
			if (( argv[2] ) && ( argv[3] ) && ( argv[4] )) {
				custom_angle = atof(argv[2]);
				custom_fx = atof(argv[3]);
				custom_fy = atof(argv[4]);
				argv += 4;
				argc -= 4;
			} else {
				fprintf(stderr,
				"The -custom option requires 3 arguments\n");
				exit(1);
			}
		} else
		if (( strcmp(argv[1], "-help") == 0 ) || (strcmp(argv[1], "--help") == 0)) {
			printf ("Usage:\n");
			printf (" -start #	Set starting test number (1=8bit, 7=24bit, 13=24bit, 20=custom)\n");
			printf (" -width #	Screen width (Default: %i)\n",w);
			printf (" -height #	Screen height (Default: %i)\n",h);
			printf (" -bpp #	Screen bpp\n");
			printf (" -warp		Enable hardware palette\n");
			printf (" -hw		Enable hardware surface\n");
			printf (" -fullscreen	Enable fullscreen mode\n");
			printf (" -custom # # #	Custom: angle, fx, fy\n");
			
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
	SDL_WM_SetCaption("SDL_rotozoom test", "rotozoom");

	/* Do all the drawing work */
	Draw (screen, start);
	
	return(0);
}
