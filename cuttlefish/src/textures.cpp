// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>

#include <circa.h>

using namespace circa;

GLenum get_texture_format(SDL_Surface *surface)
{
    assert(surface);
    int nColors = surface->format->BytesPerPixel;
    if (nColors == 4) {
        // contains alpha channel
        if (surface->format->Rmask == 0x000000ff)
            return GL_RGBA;
        else
            return GL_BGRA;
    } else if (nColors == 3) {
        // no alpha channel
        if (surface->format->Rmask == 0x000000ff)
            return GL_RGB;
        else
            return GL_BGR;
    } else {
        std::cout << "get_texture_format failed" << std::endl;
        return GL_RGBA;
    }
}

GLuint load_image_to_texture(std::string const& filename, Term* errorListener)
{
    GLuint texid;

    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (surface == NULL) {
        error_occurred(errorListener, "Error loading " + filename + ": " + SDL_GetError());
        return 0;
    }

    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel,
            surface->w, surface->h, 0,
            get_texture_format(surface), GL_UNSIGNED_BYTE, surface->pixels);

    SDL_FreeSurface(surface);

    return texid;
}

namespace textures {

void hosted_load_texture(Term* caller)
{
    if (as_int(caller) == 0) {
        std::string filename = as_string(caller->input(0));
        GLuint id = load_image_to_texture(filename, caller);
        as_int(caller) = id;
    }
}

void register_functions(circa::Branch& branch)
{
    import_function(branch, hosted_load_texture, "load_texture(string) : int");
}

} // namespace textures
