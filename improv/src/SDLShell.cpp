
#include <SDL/SDL.h>

#include "SDLShell.h"

void SDLShell::init(int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        exit(1);     
    }

    _surface = SDL_SetVideoMode(width, height, 32, SDL_OPENGL); 
    
    if (_surface == NULL) {
        printf("SDL_SetVideoMode failed: %s\n", SDL_GetError());
        exit(1);     
    }

    /*
    _cairoSurface = cairo_image_surface_create_for_data(_surface->pixels, 
        CAIRO_FORMAT_ARGB32, width, height, width * 4); 

    _cairoContext = cairo_create(_cairoSurface);
    */
}
