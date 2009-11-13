// Copyright (c) 2007-2009 Paul Hodge. All rights reserved

#include "plastic.h"

#include "sdl_utils.h"

using namespace circa;

GLenum get_texture_format(SDL_Surface *surface)
{
    assert(surface);
    int nColors = surface->format->BytesPerPixel;
    if (nColors == 4) {
        // contains alpha channel
        if (surface->format->Rmask == 0x000000ff) {
            return GL_RGBA;
        } else {
            return GL_BGRA;
        }
    } else if (nColors == 3) {
        // no alpha channel
        if (surface->format->Rmask == 0x000000ff) {
            return GL_RGB;
        } else {
            return GL_BGR;
        }
    } else {
        std::cout << "warning: get_texture_format failed, nColors = " << nColors << std::endl;
        return GL_RGBA;
    }
}

GLuint load_image_to_texture(const char* filename, Term* errorListener)
{
    SDL_Surface* surface = IMG_Load(filename);
    if (surface == NULL) {
        std::stringstream msg;
        msg << "Error loading " << filename << ": " << SDL_GetError();
        error_occurred(errorListener, msg.str());
        return 0;
    }

    if (has_indexed_color(surface))
        surface = convert_indexed_color_to_true_color(surface);

    GLuint texid = load_surface_to_texture(surface);

    SDL_FreeSurface(surface);

    return texid;
}

GLuint load_surface_to_texture(SDL_Surface *surface)
{
    GLuint texid;
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel,
            surface->w, surface->h, 0,
            get_texture_format(surface), GL_UNSIGNED_BYTE, surface->pixels);

    // reset state
    glBindTexture(GL_TEXTURE_2D, 0);

    return texid;
}

bool has_indexed_color(SDL_Surface* surface)
{
    return surface->format->BitsPerPixel == 8;
}

SDL_Color get_indexed_color(SDL_Surface* surface, int x, int y)
{
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch
        + x * surface->format->BytesPerPixel;
    int index = *p;
    return surface->format->palette->colors[index];
}

SDL_Surface* convert_indexed_color_to_true_color(SDL_Surface* surface)
{
    int width = surface->w;
    int height = surface->h;
    SDL_Surface* replacement = SDL_CreateRGBSurface(SDL_SWSURFACE,
            width, height, 24, 0, 0, 0, 0);

    for (int x=0; x < width; x++) for (int y=0; y < height; y++) {
        Uint8 *p = (Uint8*) replacement->pixels
            + y * replacement->pitch + x * 3;

        SDL_Color color = get_indexed_color(surface, x, y);

        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = color.r;
            p[1] = color.g;
            p[2] = color.b;
        } else {
            p[0] = color.b;
            p[1] = color.g;
            p[2] = color.r;
        }
    }

    SDL_FreeSurface(surface);

    return replacement;
}

