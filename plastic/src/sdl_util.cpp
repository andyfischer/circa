// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <SDL.h>

#include "gl_util.h"

GLenum get_texture_format(SDL_Surface *surface)
{
    assert(surface);
    int nColors = surface->format->BytesPerPixel;
    if (nColors == 4) {
        // Contains alpha channel
        if (surface->format->Rmask == 0x000000ff) {
            return GL_RGBA;
        } else {
            return GL_BGRA;
        }
    } else if (nColors == 3) {
        // No alpha channel
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

std::string surface_to_string(SDL_Surface* surface)
{
    std::stringstream out;
    out << "[" << surface << ", w:" << surface->w << ", h:" << surface->h
        << ", bpp:" << (int) surface->format->BytesPerPixel
        << ", Rmask:" << std::hex << surface->format->Rmask
        << ", Gmask:" << std::hex << surface->format->Gmask
        << ", Bmask:" << std::hex << surface->format->Bmask
        << ", Amask:" << std::hex << surface->format->Amask
        << ", format:" << gl_to_string(get_texture_format(surface))
        << "]";
    return out.str();
}
