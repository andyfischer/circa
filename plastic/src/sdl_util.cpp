// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <SDL.h>

#include "common_headers.h"
#include "gl_util.h"

GLenum get_texture_format(SDL_Surface *surface)
{
    ca_assert(surface);
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

GLuint load_surface_to_texture(SDL_Surface *surface)
{
    GLuint texid;
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum pixelFormat = get_texture_format(surface);

    GLenum internalFormat = (GLenum) surface->format->BytesPerPixel;

    // Specify a particular format in some cases, otherwise gl might interpret 4
    // as GL_RGB5_A1.
    switch (pixelFormat) {
        case GL_RGB: case GL_BGR: internalFormat = GL_RGB; break;
        case GL_RGBA: case GL_BGRA: internalFormat = GL_RGBA; break;
        default: break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
            surface->w, surface->h, 0,
            pixelFormat, GL_UNSIGNED_BYTE, surface->pixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texid;
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
