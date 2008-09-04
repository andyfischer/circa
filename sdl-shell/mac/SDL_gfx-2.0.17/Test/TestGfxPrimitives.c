/* 

 TestGfxPrimitives
 
 Test graphics primitive routines for accuracy (visual) and speed.

 Copyright (C) A. Schiffler, June 2001, GPL
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "SDL.h"

#include "SDL/SDL_gfxPrimitives.h"

#define WIDTH	640
#define HEIGHT	480

#define NUM_RANDOM	512

/* Coordinates */
static Sint16 rx1[NUM_RANDOM], rx2[NUM_RANDOM], ry1[NUM_RANDOM], ry2[NUM_RANDOM];

/* Radii and offsets */
static Sint16 rr1[NUM_RANDOM], rr2[NUM_RANDOM];

/* Start and stop angles */
static Sint16 a1[NUM_RANDOM], a2[NUM_RANDOM];

/* RGB colors and alpha */
static char rr[NUM_RANDOM], rg[NUM_RANDOM], rb[NUM_RANDOM], ra[NUM_RANDOM];

/* Generate an array of random screen coordinates and RGBA values with an offset */

void InitRandomPoints()
{
 int i;
 float af;

 for (i=0; i<NUM_RANDOM; i++) {
  rx1[i]=rand() % (WIDTH/2);
  rx2[i]=WIDTH/2+rx1[i];
  ry1[i]=60+(rand() % ((HEIGHT-80)/2));
  ry2[i]=20+((HEIGHT-80)/2)+ry1[i];
  rr1[i]=rand() % 32;
  rr2[i]=rand() % 32;
  a1[i]=rand() % 360;
  a2[i]=rand() % 360;
  rr[i]=rand() & 255;
  rg[i]=rand() & 255;
  rb[i]=rand() & 255;
  af=((float)rx1[i]/(float)(WIDTH/2));
  ra[i]=(int)(255.0*af);
 } 
}

#define BORDER	10

/* Set a clipping rectangle based on a rect with a border */

void SetClip (SDL_Surface *screen, int x1, int y1, int x2, int y2)
{ 
 SDL_Rect clip;
 clip.x = x1+BORDER;
 clip.y = y1+BORDER;
 clip.w = x2-x1-2*BORDER;
 clip.h = y2-y1-2*BORDER;
 SDL_SetClipRect(screen, &clip);
}

void SetClipNoBorder (SDL_Surface *screen, int x1, int y1, int x2, int y2)
{ 
 SDL_Rect clip;
 clip.x = x1;
 clip.y = y1;
 clip.w = x2-x1;
 clip.h = y2-y1;
 SDL_SetClipRect(screen, &clip);
}

void ClearScreen(SDL_Surface *screen, char *title)
{
 int x,y;
 float stepx, stepy, fx, fy, fxy;
 char titletext[256];
 Uint32 color;

 /* Setup clear color color */
 if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
  color=SDL_MapRGBA(screen->format, 0,0,0,0);
 } else {
  color=SDL_MapRGBA(screen->format, 0,0,0,0);
 }

 /* Clear the screen */
 SetClipNoBorder (screen,0,0,WIDTH-1,HEIGHT-1);
 SDL_FillRect (screen, NULL, color);

 /* Draw the colored area */
 stepx=1.0/(WIDTH/2);
 stepy=1.0/((HEIGHT-80)/2);
 fx=0.0;
 for (x=WIDTH/2; x<WIDTH; x++) {
  fy=0.0;
  for (y=(HEIGHT-40)/2+60; y<HEIGHT; y++) {
   fxy=1.0-fx*fy;
   pixelRGBA(screen,x,y,(int)(128.0*fx*fx),(int)(128.0*fxy*fxy),(int)(128.0*fy*fy),255);
   fy += stepy;
  }
  fx += stepx;
 }
 /* Segment and titles */
 hlineRGBA(screen, 0, WIDTH, 20-1, 255,255,255,255);
 hlineRGBA(screen, 0, WIDTH, 60-1, 255,255,255,255);
 hlineRGBA(screen, 0, WIDTH, (HEIGHT-40)/2+40+1, 255,255,255,255); 
 hlineRGBA(screen, 0, WIDTH, (HEIGHT-40)/2+60-1, 255,255,255,255); 
 vlineRGBA(screen, WIDTH/2, 20, HEIGHT, 255,255,255,255);
 strncpy(titletext,"Current Primitive: ",256);
 strncat(titletext,title,256);
 strncat(titletext,"  -  Click for continue. Key to Quit.",256);
 stringRGBA (screen, WIDTH/2-4*strlen(titletext),10-4,titletext,255,255,255,255);
 strncpy(titletext,"A=255 on Black",256);
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),50-4,titletext,255,255,255,255);
 strncpy(titletext,"A=0-254 on Black",256);
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),50-4,titletext,255,255,255,255);
 strncpy(titletext,"A=255, Color Test",256);
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),(HEIGHT-40)/2+50-4,titletext,255,255,255,255);
 strncpy(titletext,"A=0-254 on Color",256);
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),(HEIGHT-40)/2+50-4,titletext,255,255,255,255);
}

/* ======== Test and Benchmark routines ======= */

void TestPixel(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  pixelRGBA(screen, rx1[i], ry1[i], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  pixelRGBA(screen, rx2[i], ry1[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  pixelRGBA(screen, rx2[i], ry2[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  pixelRGBA(screen, rx1[i], ry2[i], r, g, b, 255);
 }
}

void BenchmarkPixel(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];

 /* Draw A=255 */
 repeat=200;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   pixelRGBA(screen, rx1[i], ry1[i], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=100;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   pixelRGBA(screen, rx2[i], ry1[i], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

void TestHline(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i += 2) {
  hlineRGBA(screen, rx1[i], rx1[i+1], ry1[i+1], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i += 2) {
  hlineRGBA(screen, rx2[i], rx2[i+1], ry1[i+1], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i += 2) {
  hlineRGBA(screen, rx2[i], rx2[i+1], ry2[i+1], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i += 2) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  hlineRGBA(screen, rx1[i], rx1[i]+rr1[i], ry2[i+1], r, g, b, 255);
 }
}

void BenchmarkHline(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=200;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   hlineRGBA(screen, rx1[i], rx1[i+1], ry1[i+1], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=100;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   hlineRGBA(screen, rx2[i], rx2[i+1], ry1[i+1], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);

}

void TestVline(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i += 2) {
  vlineRGBA(screen, rx1[i], ry1[i], ry1[i+1], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i += 2) {
  vlineRGBA(screen, rx2[i], ry1[i], ry1[i+1], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i += 2) {
  vlineRGBA(screen, rx2[i], ry2[i], ry2[i+1], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i += 2) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  vlineRGBA(screen, rx1[i], ry2[i], ry2[i]+rr1[i], r, g, b, 255);
 }
}

void BenchmarkVline(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=200;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   vlineRGBA(screen, rx1[i], ry1[i], ry1[i+1], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=100;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   vlineRGBA(screen, rx2[i], ry1[i], ry1[i+1], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);

}

void TestRectangle(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i += 2) {
  rectangleRGBA(screen, rx1[i], ry1[i], rx1[i+1], ry1[i+1], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i += 2) {
  rectangleRGBA(screen, rx2[i], ry1[i], rx2[i+1], ry1[i+1], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i += 2) {
  rectangleRGBA(screen, rx2[i], ry2[i], rx2[i+1], ry2[i+1], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i += 2) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  rectangleRGBA(screen, rx1[i], ry2[i], rx1[i]+rr1[i], ry2[i]+rr2[i], r, g, b, 255);
 }
}

void BenchmarkRectangle(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=200;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 2) {
   rectangleRGBA(screen, rx1[i], ry1[i], rx1[i+1], ry1[i+1], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)((NUM_RANDOM/2)*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=10;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 2) {
   rectangleRGBA(screen, rx2[i], ry1[i], rx2[i+1], ry1[i+1], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)((NUM_RANDOM/2)*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);

}

void TestBoxTwo(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();

 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  boxRGBA(screen, rx1[i], ry1[i], rx1[i]+1, ry1[i]+1, rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  boxRGBA(screen, rx2[i], ry1[i], rx2[i]+1, ry1[i]+1, rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  boxRGBA(screen, rx2[i], ry2[i], rx2[i]+1, ry2[i]+1, rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  boxRGBA(screen, rx1[i], ry2[i], rx1[i]+1, ry2[i]+1, r, g, b, 255);
 }
}

void BenchmarkBoxTwo(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=50;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   boxRGBA(screen, rx1[i], ry1[i], rx1[i]+1, ry1[i]+1, rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=50;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   boxRGBA(screen, rx2[i], ry1[i], rx2[i]+1, ry1[i]+1, rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);

}

void TestBox(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i += 2) {
  boxRGBA(screen, rx1[i], ry1[i], rx1[i+1], ry1[i+1], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i += 2) {
  boxRGBA(screen, rx2[i], ry1[i], rx2[i+1], ry1[i+1], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i += 2) {
  boxRGBA(screen, rx2[i], ry2[i], rx2[i+1], ry2[i+1], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i += 2) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  boxRGBA(screen, rx1[i], ry2[i], rx1[i]+rr1[i], ry2[i]+rr2[i], r, g, b, 255);
 }
}

void BenchmarkBox(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=10;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 2) {
   boxRGBA(screen, rx1[i], ry1[i], rx1[i+1], ry1[i+1], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)((NUM_RANDOM/2)*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=1;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 2) {
   boxRGBA(screen, rx2[i], ry1[i], rx2[i+1], ry1[i+1], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)((NUM_RANDOM/2)*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);

}

void TestLine(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i += 2) {
  lineRGBA(screen, rx1[i], ry1[i], rx1[i+1], ry1[i+1], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i += 2) {
  lineRGBA(screen, rx2[i], ry1[i], rx2[i+1], ry1[i+1], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i += 2) {
  lineRGBA(screen, rx2[i], ry2[i], rx2[i+1], ry2[i+1], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i += 2) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  lineRGBA(screen, rx1[i], ry2[i], rx1[i]+rr1[i], ry2[i]+rr2[i], r, g, b, 255);
 }
}

void BenchmarkLine(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=100;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 2) {
   lineRGBA(screen, rx1[i], ry1[i], rx1[i+1], ry1[i+1], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)((NUM_RANDOM/2)*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=5;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 2) {
   lineRGBA(screen, rx2[i], ry1[i], rx2[i+1], ry1[i+1], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)((NUM_RANDOM/2)*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);

}


void TestAALine(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i += 2) {
  aalineRGBA(screen, rx1[i], ry1[i], rx1[i+1], ry1[i+1], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i += 2) {
  aalineRGBA(screen, rx2[i], ry1[i], rx2[i+1], ry1[i+1], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i += 2) {
  aalineRGBA(screen, rx2[i], ry2[i], rx2[i+1], ry2[i+1], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i += 2) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  aalineRGBA(screen, rx1[i], ry2[i], rx1[i]+rr1[i], ry2[i]+rr2[i], r, g, b, 255);
 }
}

void BenchmarkAALine(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=5;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 2) {
   aalineRGBA(screen, rx1[i], ry1[i], rx1[i+1], ry1[i+1], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)((NUM_RANDOM/2)*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=5;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 2) {
   aalineRGBA(screen, rx2[i], ry1[i], rx2[i+1], ry1[i+1], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)((NUM_RANDOM/2)*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);

}

void TestCircle(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  circleRGBA(screen, rx1[i], ry1[i], rr1[i], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  circleRGBA(screen, rx2[i], ry1[i], rr1[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  circleRGBA(screen, rx2[i], ry2[i], rr1[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  circleRGBA(screen, rx1[i], ry2[i], rr1[i], r, g, b, 255);
 }
}

void BenchmarkCircle(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=20;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   circleRGBA(screen, rx1[i], ry1[i], rr1[i], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=5;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   circleRGBA(screen, rx2[i], ry1[i], rr1[i], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

void TestAACircle(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  aacircleRGBA(screen, rx1[i], ry1[i], rr1[i], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  aacircleRGBA(screen, rx2[i], ry1[i], rr1[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  aacircleRGBA(screen, rx2[i], ry2[i], rr1[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  aacircleRGBA(screen, rx1[i], ry2[i], rr1[i], r, g, b, 255);
 }
}

void BenchmarkAACircle(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=20;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   aacircleRGBA(screen, rx1[i], ry1[i], rr1[i], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=5;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   aacircleRGBA(screen, rx2[i], ry1[i], rr1[i], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

void TestFilledCircle(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  filledCircleRGBA(screen, rx1[i], ry1[i], rr1[i], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  filledCircleRGBA(screen, rx2[i], ry1[i], rr1[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  filledCircleRGBA(screen, rx2[i], ry2[i], rr1[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  filledCircleRGBA(screen, rx1[i], ry2[i], rr1[i], r, g, b, 255);
 }
}

void BenchmarkFilledCircle(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=20;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   filledCircleRGBA(screen, rx1[i], ry1[i], rr1[i], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=5;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   filledCircleRGBA(screen, rx2[i], ry1[i], rr1[i], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

void TestEllipse(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  ellipseRGBA(screen, rx1[i], ry1[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  ellipseRGBA(screen, rx2[i], ry1[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  ellipseRGBA(screen, rx2[i], ry2[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  ellipseRGBA(screen, rx1[i], ry2[i], rr1[i], rr2[i], r, g, b, 255);
 }
}

void BenchmarkEllipse(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=20;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   ellipseRGBA(screen, rx1[i], ry1[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=5;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   ellipseRGBA(screen, rx2[i], ry1[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

void TestAAEllipse(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  aaellipseRGBA(screen, rx1[i], ry1[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  aaellipseRGBA(screen, rx2[i], ry1[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  aaellipseRGBA(screen, rx2[i], ry2[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  aaellipseRGBA(screen, rx1[i], ry2[i], rr1[i], rr2[i], r, g, b, 255);
 }
}

void BenchmarkAAEllipse(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=20;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   ellipseRGBA(screen, rx1[i], ry1[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=5;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   aaellipseRGBA(screen, rx2[i], ry1[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

void TestFilledEllipse(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  filledEllipseRGBA(screen, rx1[i], ry1[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  filledEllipseRGBA(screen, rx2[i], ry1[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  filledEllipseRGBA(screen, rx2[i], ry2[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  filledEllipseRGBA(screen, rx1[i], ry2[i], rr1[i], rr2[i], r, g, b, 255);
 }
}

void BenchmarkFilledEllipse(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=20;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   filledEllipseRGBA(screen, rx1[i], ry1[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=5;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   filledEllipseRGBA(screen, rx2[i], ry1[i], rr1[i], rr2[i], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

void TestArc(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  arcRGBA(screen, rx1[i], ry1[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  arcRGBA(screen, rx2[i], ry1[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  arcRGBA(screen, rx2[i], ry2[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  arcRGBA(screen, rx1[i], ry2[i], rr1[i], a1[i], a2[i], r, g, b, 255);
 }
}

void BenchmarkArc(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=50;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   arcRGBA(screen, rx1[i], ry1[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=5;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   arcRGBA(screen, rx2[i], ry1[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

void TestPie(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  pieRGBA(screen, rx1[i], ry1[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  pieRGBA(screen, rx2[i], ry1[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  pieRGBA(screen, rx2[i], ry2[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  pieRGBA(screen, rx1[i], ry2[i], rr1[i], a1[i], a2[i], r, g, b, 255);
 }
}

void BenchmarkPie(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=20;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   pieRGBA(screen, rx1[i], ry1[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=5;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   pieRGBA(screen, rx2[i], ry1[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

void TestFilledPie(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  filledPieRGBA(screen, rx1[i], ry1[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<NUM_RANDOM; i++) {
  filledPieRGBA(screen, rx2[i], ry1[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  filledPieRGBA(screen, rx2[i], ry2[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<NUM_RANDOM; i++) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  filledPieRGBA(screen, rx1[i], ry2[i], rr1[i], a1[i], a2[i], r, g, b, 255);
 }
}

void BenchmarkFilledPie(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 
 /* Draw A=255 */
 repeat=20;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   filledPieRGBA(screen, rx1[i], ry1[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=5;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i++) {
   filledPieRGBA(screen, rx2[i], ry1[i], rr1[i], a1[i], a2[i], rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(NUM_RANDOM*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

void TestPolygon(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  polygonRGBA(screen, &rx1[i], &ry1[i], 3, rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  polygonRGBA(screen, &rx2[i], &ry1[i], 3, rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  polygonRGBA(screen, &rx2[i], &ry2[i], 3, rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  rx1[i+1]=rx1[i]+rr1[i];
  rx1[i+2]=rx1[i];
  ry2[i+1]=ry2[i];
  ry2[i+2]=ry2[i]+rr2[i];
  polygonRGBA(screen, &rx1[i], &ry2[i], 3, r, g, b, 255);
 }
}

void BenchmarkPolygon(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];

 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 repeat=50;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 3) {
   polygonRGBA(screen, &rx1[i], &ry1[i], 3, rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(((NUM_RANDOM-3)/3)*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=10;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 3) {
   polygonRGBA(screen, &rx2[i], &ry1[i], 3, rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(((NUM_RANDOM-3)/3)*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

void TestAAPolygon(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  aapolygonRGBA(screen, &rx1[i], &ry1[i], 3, rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  aapolygonRGBA(screen, &rx2[i], &ry1[i], 3, rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  aapolygonRGBA(screen, &rx2[i], &ry2[i], 3, rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  rx1[i+1]=rx1[i]+rr1[i];
  rx1[i+2]=rx1[i];
  ry2[i+1]=ry2[i];
  ry2[i+2]=ry2[i]+rr2[i];
  aapolygonRGBA(screen, &rx1[i], &ry2[i], 3, r, g, b, 255);
 }
}

void BenchmarkAAPolygon(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];

 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 repeat=5;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 3) {
   aapolygonRGBA(screen, &rx1[i], &ry1[i], 3, rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(((NUM_RANDOM-3)/3)*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=2;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 3) {
   aapolygonRGBA(screen, &rx2[i], &ry1[i], 3, rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(((NUM_RANDOM-3)/3)*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}


void TestFilledPolygon(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  filledPolygonRGBA(screen, &rx1[i], &ry1[i], 3, rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  filledPolygonRGBA(screen, &rx2[i], &ry1[i], 3, rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  filledPolygonRGBA(screen, &rx2[i], &ry2[i], 3, rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  rx1[i+1]=rx1[i]+rr1[i];
  rx1[i+2]=rx1[i];
  ry2[i+1]=ry2[i];
  ry2[i+2]=ry2[i]+rr2[i];
  filledPolygonRGBA(screen, &rx1[i], &ry2[i], 3, r, g, b, 255);
 }
}

void BenchmarkFilledPolygon(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];

 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 repeat=20;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 3) {
   filledPolygonRGBA(screen, &rx1[i], &ry1[i], 3, rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(((NUM_RANDOM-3)/3)*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=5;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 3) {
   filledPolygonRGBA(screen, &rx2[i], &ry1[i], 3, rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(((NUM_RANDOM-3)/3)*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

void TestTexturedPolygon(SDL_Surface *screen)
{
 int i;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 SDL_Surface *texture;

 /* Load texture surface */
 texture = SDL_LoadBMP("texture.bmp");
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<(NUM_RANDOM/2-3); i += 3) {
  texturedPolygon(screen, &rx1[i], &ry1[i], 3, texture, rr1[i], rr2[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<(NUM_RANDOM/2-3); i += 3) {
  SDL_SetAlpha(texture, SDL_SRCALPHA, ra[i]);
  texturedPolygon(screen, &rx2[i], &ry1[i], 3, texture, rr1[i], rr2[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<(NUM_RANDOM/2-3); i += 3) {
  texturedPolygon(screen, &rx2[i], &ry2[i], 3, texture, rr1[i], rr2[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<(NUM_RANDOM/2-3); i += 3) {
  rx1[i+1]=rx1[i]+rr1[i];
  rx1[i+2]=rx1[i];
  ry2[i+1]=ry2[i];
  ry2[i+2]=ry2[i]+rr2[i];
  texturedPolygon(screen, &rx1[i], &ry2[i], 3, texture, 0, 0);
 }

}

void BenchmarkTexturedPolygon(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];
 SDL_Surface *texture;

 /* Load texture surface */
 texture = SDL_LoadBMP("texture.bmp");

 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 repeat=20;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<(NUM_RANDOM/2-3); i += 3) {
   texturedPolygon(screen, &rx1[i], &ry1[i], 3, texture, rr1[i], rr2[i]);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(((NUM_RANDOM/2-3)/3)*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=5;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<(NUM_RANDOM/2-3); i += 3) {
   SDL_SetAlpha(texture, SDL_SRCALPHA, ra[i]);
   texturedPolygon(screen, &rx2[i], &ry1[i], 3, texture, rr1[i], rr2[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(((NUM_RANDOM/2-3)/3)*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

void TestBezier(SDL_Surface *screen)
{
 int i;
 char r,g,b;
 
 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  bezierRGBA(screen, &rx1[i], &ry1[i], 3, 100, rr[i], rg[i], rb[i], 255);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  bezierRGBA(screen, &rx2[i], &ry1[i], 3, 100, rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw A=various */
 SetClip(screen,WIDTH/2,80+(HEIGHT-80)/2,WIDTH,HEIGHT);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  bezierRGBA(screen, &rx2[i], &ry2[i], 3, 100, rr[i], rg[i], rb[i], ra[i]);
 }

 /* Draw Colortest */
 SetClip(screen,0,80+(HEIGHT-80)/2,WIDTH/2,HEIGHT);
 for (i=0; i<(NUM_RANDOM-3); i += 3) {
  if (rx1[i] < (WIDTH/6))  {
   r=255; g=0; b=0; 
  } else if (rx1[i] < (WIDTH/3) ) {
   r=0; g=255; b=0; 
  } else {
   r=0; g=0; b=255; 
  }
  rx1[i+1]=rx1[i]+rr1[i];
  rx1[i+2]=rx1[i];
  ry2[i+1]=ry2[i];
  ry2[i+2]=ry2[i]+rr2[i];
  bezierRGBA(screen, &rx1[i], &ry2[i], 3, 100, r, g, b, 255);
 }
}

void BenchmarkBezier(SDL_Surface *screen)
{
 int i,j;
 int repeat;
 Uint32 time1, time2;
 char titletext[256];

 /* Create random points */
 srand(time(NULL));
 InitRandomPoints();
 
 /* Draw A=255 */
 repeat=20;
 SetClip(screen,0,60,WIDTH/2,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 3) {
   bezierRGBA(screen, &rx1[i], &ry1[i], 3, 100, rr[i], rg[i], rb[i], 255);
  }
 }
 time2=SDL_GetTicks();
 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(((NUM_RANDOM-3)/3)*repeat)/(float)(time2-time1));
 stringRGBA (screen, WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
 
 /* Draw A=various */
 repeat=10;
 SetClip(screen,WIDTH/2,60,WIDTH,60+(HEIGHT-80)/2);
 time1=SDL_GetTicks();
 for (j=0; j<repeat; j++) {
  for (i=0; i<NUM_RANDOM; i += 3) {
   bezierRGBA(screen, &rx2[i], &ry1[i], 3, 100, rr[i], rg[i], rb[i], ra[i]);
  }
 }
 time2=SDL_GetTicks();

 /* Results */
 SetClip(screen,0,0,WIDTH-1,HEIGHT-1);
 sprintf (titletext, "%.0f per sec",1000.0*(float)(((NUM_RANDOM-3)/3)*repeat)/(float)(time2-time1));
 stringRGBA (screen, 3*WIDTH/4-4*strlen(titletext),30-4,titletext,255,255,255,255);
}

/* ======== Main Program ======= */

int main(int argc, char *argv[])
{
	const SDL_VideoInfo *info;
	SDL_Surface *screen;
	Uint8  video_bpp;
	Uint32 videoflags;
	int done;
	SDL_Event event;
	int oldprim, curprim;
        char title[64];
        
        /* Generate title string */
        sprintf (title,"TestGfxPrimitives - Ver. %i.%i.%i",SDL_GFXPRIMITIVES_MAJOR, SDL_GFXPRIMITIVES_MINOR, SDL_GFXPRIMITIVES_MICRO);
        
	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	/* Alpha blending doesn't work well at 8-bit color */
	info = SDL_GetVideoInfo();
	if ( info->vfmt->BitsPerPixel > 8 ) {
		video_bpp = info->vfmt->BitsPerPixel;
	} else {
		video_bpp = 16;
	}
	videoflags = SDL_SWSURFACE | SDL_SRCALPHA | SDL_RESIZABLE;
	while ( argc > 1 ) {
		--argc;
		if ( strcmp(argv[argc-1], "-bpp") == 0 ) {
			video_bpp = atoi(argv[argc]);
			--argc;
		} else
		if ( strcmp(argv[argc], "-hw") == 0 ) {
			videoflags |= SDL_HWSURFACE;
		} else
		if ( strcmp(argv[argc], "-warp") == 0 ) {
			videoflags |= SDL_HWPALETTE;
		} else
		if ( strcmp(argv[argc], "-fullscreen") == 0 ) {
			videoflags |= SDL_FULLSCREEN;
		} else {
			fprintf(stderr, 
			"Usage: %s [-bpp N] [-warp] [-hw] [-fullscreen]\n",
								argv[0]);
			exit(1);
		}
	}

	/* Set 640x480 video mode */
	if ( (screen=SDL_SetVideoMode(WIDTH,HEIGHT,video_bpp,videoflags)) == NULL ) {
		fprintf(stderr, "Couldn't set %ix%i video mode: %s\n",WIDTH,HEIGHT,SDL_GetError());
		exit(2);
	}


	
	/* Use alpha blending */
	SDL_SetAlpha(screen, SDL_SRCALPHA, 0);
	
	/* Set title for window */
 	SDL_WM_SetCaption(title,title);
 	
	/* Wait for a keystroke */
	done = 0;
	oldprim = 0;
	curprim = 1;
	while ( !done ) {

		/* Draw on screen if primitive changes */
		if (curprim != oldprim) {
		
			ClearScreen(screen, "Titletext");

			/* Draw according to curprim setting */
			switch (curprim) {
			
			 /* Pixels */
			 case 1:
		 	  ClearScreen(screen, "Pixel");
			  TestPixel(screen);
			  BenchmarkPixel(screen);
 			  /* Next primitive */
	 		  oldprim=curprim; 
			  break;
			 
			 /* Horizontal Lines */
			 case 2:
		 	  ClearScreen(screen, "Hline");
			  TestHline(screen);
			  BenchmarkHline(screen);
 			  /* Next primitive */
	 		  oldprim=curprim; 
 			  break;
 			  
 			 /* Vertical Lines */
			 case 3:
		 	  ClearScreen(screen, "Vline");
			  TestVline(screen);
			  BenchmarkVline(screen);
 			  /* Next primitive */
	 		  oldprim=curprim; 
			  break;
			 
			 /* Rectangles */
			 case 4:
		 	  ClearScreen(screen, "Rectangle");
			  TestRectangle(screen);
			  BenchmarkRectangle(screen);
 			  /* Next primitive */
	 		  oldprim=curprim; 
			  break;

			 /* --- Box */
			 case 5:
		 	  ClearScreen(screen, "Box");
			  TestBox(screen);
			  BenchmarkBox(screen);
 			  /* Next primitive */ 			  
	 		  oldprim=curprim; 
			  break;

			 /* --- 2x2 Box */
			 case 6:
		 	  ClearScreen(screen, "2x2 Box");
			  TestBoxTwo(screen);
			  BenchmarkBoxTwo(screen);
 			  /* Next primitive */ 			  
	 		  oldprim=curprim; 
			  break;
			  
			 /* --- Lines */
			 case 7:
		 	  ClearScreen(screen, "Line");
			  TestLine(screen);
			  BenchmarkLine(screen);
 			  /* Next primitive */
	 		  oldprim=curprim; 
			  break;

			 /* --- AA Line */
			 case 8:
		 	  ClearScreen(screen, "Anti-Aliased Line");
			  TestAALine(screen);
			  BenchmarkAALine(screen);
 			  /* Next primitive */
	 		  oldprim=curprim; 
			  break;
			  
			 /* --- Circle */
			 case 9:
		 	  ClearScreen(screen, "Circle");
			  TestCircle(screen);
			  BenchmarkCircle(screen);			 
 			  /* Next primitive */
	 		  oldprim=curprim; 
			  break;

			 /* --- AA Circle */
			 case 10:
		 	  ClearScreen(screen, "AA Circle");
			  TestAACircle(screen);
			  BenchmarkAACircle(screen);			 
 			  /* Next primitive */
	 		  oldprim=curprim; 
			  break;
			  
			 /* --- Filled Circle */
			 case 11:
		 	  ClearScreen(screen, "Filled Circle");
			  TestFilledCircle(screen);
			  BenchmarkFilledCircle(screen);
 			  /* Next primitive */
	 		  oldprim=curprim; 
			  break;

			 /* --- Ellipse */
			 case 12:
		 	  ClearScreen(screen, "Ellipse");
			  TestEllipse(screen);
			  BenchmarkEllipse(screen);
 			  /* Next primitive */
	 		  oldprim=curprim; 
			  break;

			 /* --- AA Ellipse */
			 case 13:
		 	  ClearScreen(screen, "AA Ellipse");
			  TestAAEllipse(screen);
			  BenchmarkAAEllipse(screen);
 			  /* Next primitive */
	 		  oldprim=curprim; 
			  break;

			 /* --- Filled Ellipse */
			 case 14:
		 	  ClearScreen(screen, "Filled Ellipse");
			  TestFilledEllipse(screen);
			  BenchmarkFilledEllipse(screen);
 			  /* Next primitive */ 			  
	 		  oldprim=curprim; 
			  break;

			 /* --- Arc */
			 case 15:
		 	  ClearScreen(screen, "Arc");
			  TestArc(screen);
			  BenchmarkArc(screen);
 			  /* Next primitive */ 			  
	 		  oldprim=curprim; 
			  break;
			  
			 /* --- Pie */
			 case 16:
		 	  ClearScreen(screen, "Pie");
			  TestPie(screen);
			  BenchmarkPie(screen);
 			  /* Next primitive */ 			  
	 		  oldprim=curprim; 
			  break;

			 /* --- Filled Pie */
			 case 17:
		 	  ClearScreen(screen, "Filled Pie");
			  TestFilledPie(screen);
			  BenchmarkFilledPie(screen);
 			  /* Next primitive */ 			  
	 		  oldprim=curprim; 
			  break;

			 /* --- Polygon */
			 case 18:
		 	  ClearScreen(screen, "Polygon");
			  TestPolygon(screen);
			  BenchmarkPolygon(screen);
 			  /* Next primitive */ 			  
	 		  oldprim=curprim; 
			  break;
			 
			 /* --- AA-Polygon */
			 case 19:
		 	  ClearScreen(screen, "AA-Polygon");
			  TestAAPolygon(screen);
			  BenchmarkAAPolygon(screen);
 			  /* Next primitive */ 			  
	 		  oldprim=curprim; 
			  break;
			 
			 /* ---- Filled Polygon */ 
			 case 20:
		 	  ClearScreen(screen, "Filled Polygon");
			  TestFilledPolygon(screen);
			  BenchmarkFilledPolygon(screen);
 			  /* Next primitive */ 			  
	 		  oldprim=curprim; 
			  break;

			 /* ---- Textured Polygon */ 
			 case 21:
		 	  ClearScreen(screen, "Textured Polygon");
			  TestTexturedPolygon(screen);
			  BenchmarkTexturedPolygon(screen);
 			  /* Next primitive */ 			  
	 		  oldprim=curprim; 
			  break;

			 /* ---- Bezier Curve */ 
			 case 22:
		 	  ClearScreen(screen, "Bezier Curve");
			  TestBezier(screen);
			  BenchmarkBezier(screen);
 			  /* Next primitive */ 			  
	 		  oldprim=curprim; 
			  break;
			  
			 /* --- Reset */ 
			 default:
			  oldprim=0;
			  curprim=1;
			  break;
			}
			
			/* Show */
			SDL_UpdateRect(screen, 0, 0, 0, 0);
			SDL_Flip(screen);
		}
		
		/* Slow down polling */
		SDL_Delay(100);
		
		/* Check for events */
		while ( SDL_PollEvent(&event) ) {
			switch (event.type) {
				case SDL_MOUSEBUTTONDOWN:
					if ( event.button.button == 1 ) {
						/* Switch to next graphics */
						curprim++;
					}
					break;
				case SDL_KEYDOWN:
					/* Any keypress quits the app... */
				case SDL_QUIT:
					done = 1;
					break;
				default:
					break;
			}
		}
	}

	return(0);
}
