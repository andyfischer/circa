// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <SDL_image.h>

#include "circa.h"

bool has_indexed_color(SDL_Surface* surface)
{
    return surface->format->BitsPerPixel == 8;
}

unsigned int get_color_index(SDL_Surface* surface, int x, int y)
{
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch
        + x * surface->format->BytesPerPixel;
    return *p;
}

SDL_Surface* convert_indexed_color_to_true_color(SDL_Surface* surface)
{
    int width = surface->w;
    int height = surface->h;
    SDL_Surface* replacement = SDL_CreateRGBSurface(SDL_SWSURFACE,
            width, height, 32, 0, 0, 0, 0);

    for (int x=0; x < width; x++) for (int y=0; y < height; y++) {
        Uint8 *p = (Uint8*) replacement->pixels
            + y * replacement->pitch + x * 4;

        unsigned int index = get_color_index(surface, x, y);
        SDL_Color color = surface->format->palette->colors[index];

        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = color.r;
            p[1] = color.g;
            p[2] = color.b;
        } else {
            p[0] = color.b;
            p[1] = color.g;
            p[2] = color.r;
        }
        
        // alpha channel
        if (index == surface->format->colorkey)
            p[3] = 0x0;
        else
            p[3] = 0xff;
    }

    SDL_FreeSurface(surface);

    return replacement;
}

CA_FUNCTION(load_image)
{
    const char* filename = STRING_INPUT(0);

    SDL_Surface* surface = IMG_Load(filename);
    if (surface == NULL) {
        std::stringstream msg;
        msg << "Error loading " << filename << ": " << SDL_GetError();
        return error_occurred(CONTEXT, CALLER, msg.str().c_str());
    }

    if (has_indexed_color(surface))
        surface = convert_indexed_color_to_true_color(surface);

    set_list(OUTPUT, 4);

#if 0
TODO
    image_t::set_filename(OUTPUT, filename);
    image_t::set_texid(OUTPUT, load_surface_to_texture(surface));
    image_t::set_width(OUTPUT, surface->w);
    image_t::set_height(OUTPUT, surface->h);
#endif

    SDL_FreeSurface(surface);
}
