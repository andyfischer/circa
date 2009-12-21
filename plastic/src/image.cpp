// Copyright (c) 2007-2009 Paul Hodge. All rights reserved

#include <circa.h>

#include "plastic.h"

using namespace circa;

struct Image
{
    Term* _term;
    Image(Term* term) : _term(term) {}

    std::string& filename() { return _term->asBranch()[0]->asString(); }
    int& texid() { return _term->asBranch()[1]->asInt(); }
    int& width() { return _term->asBranch()[2]->asInt(); }
    int& height() { return _term->asBranch()[3]->asInt(); }
};

struct Point
{
    Term* _term;
    Point(Term* term) : _term(term) {}

    float x() { return _term->asBranch()[0]->toFloat(); }
    float y() { return _term->asBranch()[1]->toFloat(); }
};

struct Rect
{
    Term* _term;
    Rect(Term* term) : _term(term) {}

    float x1() { return _term->asBranch()[0]->toFloat(); }
    float y1() { return _term->asBranch()[1]->toFloat(); }
    float x2() { return _term->asBranch()[2]->toFloat(); }
    float y2() { return _term->asBranch()[3]->toFloat(); }
};

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

    glBindTexture(GL_TEXTURE_2D, 0);

    return texid;
}

bool has_indexed_color(SDL_Surface* surface)
{
    return surface->format->BitsPerPixel == 8;
}

int get_color_index(SDL_Surface* surface, int x, int y)
{
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch
        + x * surface->format->BytesPerPixel;
    return *p;
}

SDL_Color get_color_from_index(SDL_Surface* surface, int index) {
    return surface->format->palette->colors[index];
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

        int index = get_color_index(surface, x, y);
        SDL_Color color = get_color_from_index(surface, index);

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

void load_image(Term* caller)
{
    std::string& filename = caller->input(0)->asString();
    Image result(caller);

    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (surface == NULL) {
        error_occurred(caller, "Error loading " + filename + ": " + SDL_GetError());
        return;
    }

    if (has_indexed_color(surface))
        surface = convert_indexed_color_to_true_color(surface);

    result.texid() = load_surface_to_texture(surface);
    result.width() = surface->w;
    result.height() = surface->h;
    result.filename() = filename;

    SDL_FreeSurface(surface);
}

void draw_image(Term* caller)
{
    Image image(caller->input(0));
    Point point(caller->input(1));

    float x = point.x();
    float y = point.y();
    float width = float(image.width());
    float height = float(image.height());

    glBindTexture(GL_TEXTURE_2D, image.texid());
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex3f(x, y, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(x + width, y, 0);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(x + width, y + height, 0);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(x, y + height, 0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);

    gl_check_error(caller);
}

void draw_image_clip(Term* caller)
{
    Image image(caller->input(0));
    Rect clip(caller->input(1));

    Branch& destination = caller->input(2)->asBranch();
    float dest_x = destination[0]->asFloat();
    float dest_y = destination[1]->asFloat();

    float dest_x2, dest_y2;

    if (destination.length() > 2) {
        dest_x2 = destination[2]->asFloat();
        dest_y2 = destination[3]->asFloat();
    } else {
        dest_x2 = dest_x + clip.x2() - clip.x1();
        dest_y2 = dest_y + clip.y2() - clip.y1();
    }

    float tex_x1 = clip.x1() / image.width();
    float tex_x2 = clip.x2() / image.width();
    float tex_y1 = clip.y1() / image.height();
    float tex_y2 = clip.y2() / image.height();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glBindTexture(GL_TEXTURE_2D, image.texid());
    glBegin(GL_QUADS);
    glTexCoord2d(tex_x1, tex_y1);
    glVertex3f(dest_x, dest_y, 0);
    glTexCoord2d(tex_x2, tex_y1);
    glVertex3f(dest_x2, dest_y, 0);
    glTexCoord2d(tex_x2, tex_y2);
    glVertex3f(dest_x2, dest_y2, 0);
    glTexCoord2d(tex_x1, tex_y2);
    glVertex3f(dest_x, dest_y2, 0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);

    gl_check_error(caller);
}

namespace image {

void setup(circa::Branch& branch)
{
    Branch& image_ns = branch["image"]->asBranch();
    install_function(image_ns["_load"], load_image);
    install_function(image_ns["draw"], draw_image);
    install_function(image_ns["draw_clip_p"], draw_image_clip);
    install_function(image_ns["draw_clip_resized"], draw_image_clip);
}

} // namespace image
