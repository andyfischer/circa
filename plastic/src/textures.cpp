// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>

#include <circa.h>

#include "textures.h"

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

GLuint load_image_to_texture(std::string const& filename, Term* errorListener)
{
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (surface == NULL) {
        error_occurred(errorListener, "Error loading " + filename + ": " + SDL_GetError());
        return 0;
    }

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

namespace textures {

void hosted_load_texture(Term* caller)
{
    int& texid = caller->input(0)->asInt();

    if (texid == 0) {
        std::string filename = caller->input(1)->asString();
        GLuint id = load_image_to_texture(filename, caller);
        texid = id;
    }
    caller->asInt() = texid;
}

void hosted_image(Term* caller)
{
    int& texid = caller->input(0)->asInt();
    std::string filename = caller->input(1)->asString();
    float x1 = caller->input(2)->toFloat();
    float y1 = caller->input(3)->toFloat();
    float x2 = caller->input(4)->toFloat();
    float y2 = caller->input(5)->toFloat();

    if (texid == 0) {
        texid = load_image_to_texture(filename, caller);
        if (caller->hasError) return;
    }

    glBindTexture(GL_TEXTURE_2D, texid);
    glColor4f(1,1,1,1);

    glBegin(GL_QUADS);

    glTexCoord2d(0.0, 0.0);
    glVertex3f(x1, y1, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(x2, y1,0);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(x2, y2,0);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(x1, y2,0);

    glEnd();

    // reset state
    glBindTexture(GL_TEXTURE_2D, 0);
}

void setup(circa::Branch& branch)
{
    install_function(branch["load_texture"], hosted_load_texture);
    install_function(branch["draw_image"], hosted_image);
}

} // namespace textures
