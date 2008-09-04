/* 

 TestABGR
 
 Test GFX behavior on byteordering

 (C) 2005, GPL
 
*/


#include "SDL.h"
#include "SDL/SDL_gfxPrimitives.h"

int main(int argc, char** argv)
{
  SDL_Rect r;

  /* Basic 32bit screen */
  SDL_Init(SDL_INIT_VIDEO);
  SDL_SetVideoMode(400, 400, 32, 0);

  /* Draw some white stripes as background */
  int y;
  for (y=0; y<400; y += 20) {
   boxRGBA(SDL_GetVideoSurface(), 0, y, 400, y+10, 255, 255, 255, 255);
  }
  
  /* Define masking bytes */
  Uint32 mask1 = 0xff000000; 
  Uint32 mask2 = 0x00ff0000;
  Uint32 mask3 = 0x0000ff00; 
  Uint32 mask4 = 0x000000ff;
 
  /* Solid color test */
  {
  SDL_Surface* s1 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask1, mask2, mask3, mask4);
  filledEllipseRGBA(s1, 0, 0, 100, 100, 255, 0, 0, 255); // red
  r.x = 0; r.y = 0;
  SDL_BlitSurface(s1, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s2 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask1, mask2, mask3, mask4);
  filledEllipseRGBA(s2, 0, 0, 100, 100, 0, 255, 0, 255); // green
  r.x = 0; r.y = 100;
  SDL_BlitSurface(s2, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s3 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask1, mask2, mask3, mask4);
  filledEllipseRGBA(s3, 0, 0, 100, 100, 0, 0, 255, 255); // blue
  r.x = 100; r.y = 0;
  SDL_BlitSurface(s3, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s4 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask1, mask2, mask3, mask4);
  filledEllipseRGBA(s4, 0, 0, 100, 100, 255, 255, 255, 255); // white
  r.x = 100; r.y = 100;
  SDL_BlitSurface(s4, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s5 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask4, mask3, mask2, mask1);
  filledEllipseRGBA(s5, 0, 0, 100, 100, 255, 0, 0, 255); // red
  r.x = 200; r.y = 0;
  SDL_BlitSurface(s5, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s6 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask4, mask3, mask2, mask1);
  filledEllipseRGBA(s6, 0, 0, 100, 100, 0, 255, 0, 255); // green
  r.x = 200; r.y = 100;
  SDL_BlitSurface(s6, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s7 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask4, mask3, mask2, mask1);
  filledEllipseRGBA(s7, 0, 0, 100, 100, 0, 0, 255, 255); // blue
  r.x = 300; r.y = 0;
  SDL_BlitSurface(s7, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s8 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask4, mask3, mask2, mask1);
  filledEllipseRGBA(s8, 0, 0, 100, 100, 255, 255, 255, 255); // white
  r.x = 300; r.y = 100;
  SDL_BlitSurface(s8, 0, SDL_GetVideoSurface(), &r);
  }
 
  /* Transparent color test */
  {
  SDL_Surface* s1 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask1, mask2, mask3, mask4);
  filledEllipseRGBA(s1, 0, 0, 100, 100, 255, 0, 0, 200); // red+trans
  r.x = 0; r.y = 200;
  SDL_BlitSurface(s1, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s2 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask1, mask2, mask3, mask4);
  filledEllipseRGBA(s2, 0, 0, 100, 100, 0, 255, 0, 200); // green+trans
  r.x = 0; r.y = 300;
  SDL_BlitSurface(s2, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s3 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask1, mask2, mask3, mask4);
  filledEllipseRGBA(s3, 0, 0, 100, 100, 0, 0, 255, 200); // blue+trans
  r.x = 100; r.y = 200;
  SDL_BlitSurface(s3, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s4 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask1, mask2, mask3, mask4);
  filledEllipseRGBA(s4, 0, 0, 100, 100, 255, 255, 255, 200); // white+trans
  r.x = 100; r.y = 300;
  SDL_BlitSurface(s4, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s5 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask4, mask3, mask2, mask1);
  filledEllipseRGBA(s5, 0, 0, 100, 100, 255, 0, 0, 200); // red+trans
  r.x = 200; r.y = 200;
  SDL_BlitSurface(s5, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s6 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask4, mask3, mask2, mask1);
  filledEllipseRGBA(s6, 0, 0, 100, 100, 0, 255, 0, 200); // green+trans
  r.x = 200; r.y = 300;
  SDL_BlitSurface(s6, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s7 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask4, mask3, mask2, mask1);
  filledEllipseRGBA(s7, 0, 0, 100, 100, 0, 0, 255, 200); // blue+trans
  r.x = 300; r.y = 200;
  SDL_BlitSurface(s7, 0, SDL_GetVideoSurface(), &r);

  SDL_Surface* s8 =
    SDL_CreateRGBSurface(0, 200, 200, 32, mask4, mask3, mask2, mask1);
  filledEllipseRGBA(s8, 0, 0, 100, 100, 255, 255, 255, 200); // white+trans
  r.x = 300; r.y = 300;
  SDL_BlitSurface(s8, 0, SDL_GetVideoSurface(), &r);
  }
  SDL_Flip(SDL_GetVideoSurface());

  printf ("Left - RGBA\nRight - ABGR\nTop - Solid\nBottom - Transparent\n");
  
  SDL_Event e;
  for(;;) {
    SDL_PollEvent(&e);
    usleep(1000);
    if ((e.type == SDL_QUIT) || (e.type == SDL_KEYDOWN)) break;
  }

}
