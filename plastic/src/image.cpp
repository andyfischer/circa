// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>

#include "plastic.h"

using namespace circa;

namespace image_t {
    std::string get_filename(Term* term) { return term->asBranch()[0]->asString(); }
    int get_texid(Term* term) { return term->asBranch()[1]->asInt(); }
    int get_width(Term* term) { return term->asBranch()[2]->asInt(); }
    int get_height(Term* term) { return term->asBranch()[3]->asInt(); }

    void set_filename(Term* term, std::string const& s) { set_str(term->asBranch()[0], s); }
    void set_texid(Term* term, int id) { set_int(term->asBranch()[1], id); }
    void set_width(Term* term, int w) { set_int(term->asBranch()[2], w); }
    void set_height(Term* term, int h) { set_int(term->asBranch()[3], h); }
}

namespace rect_t {
    float get_x1(Term* term) { return term->asBranch()[0]->toFloat(); }
    float get_y1(Term* term) { return term->asBranch()[1]->toFloat(); }
    float get_x2(Term* term) { return term->asBranch()[2]->toFloat(); }
    float get_y2(Term* term) { return term->asBranch()[3]->toFloat(); }
}

GLuint load_image_to_texture(EvalContext* cxt, Term* term, const char* filename)
{
    SDL_Surface* surface = IMG_Load(filename);
    if (surface == NULL) {
        std::stringstream msg;
        msg << "Error loading " << filename << ": " << SDL_GetError();
        error_occurred(cxt, term, msg.str());
        return 0;
    }

    if (has_indexed_color(surface))
        surface = convert_indexed_color_to_true_color(surface);

    GLuint texid = load_surface_to_texture(surface);

    SDL_FreeSurface(surface);

    return texid;
}

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

void load_image(EvalContext* cxt, Term* caller)
{
    std::string const& filename = caller->input(0)->asString();

    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (surface == NULL) {
        error_occurred(cxt, caller, "Error loading " + filename + ": " + SDL_GetError());
        return;
    }

    if (has_indexed_color(surface))
        surface = convert_indexed_color_to_true_color(surface);

    image_t::set_texid(caller, load_surface_to_texture(surface));
    image_t::set_filename(caller, filename);
    image_t::set_width(caller, surface->w);
    image_t::set_height(caller, surface->h);

    SDL_FreeSurface(surface);
}

void draw_image(EvalContext* cxt, Term* caller)
{
    Term* image = caller->input(0);
    Term* point = caller->input(1);

    float x = 0;
    float y = 0;
    circa::point_t::read(point, &x, &y);
    float width = float(image_t::get_width(image));
    float height = float(image_t::get_height(image));

    glBindTexture(GL_TEXTURE_2D, image_t::get_texid(image));
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

    gl_check_error(cxt, caller);
}

void draw_image_clip(EvalContext* cxt, Term* caller)
{
    Term* image = caller->input(0);
    Term* clip = caller->input(1);

    Branch& destination = caller->input(2)->asBranch();
    float dest_x = destination[0]->asFloat();
    float dest_y = destination[1]->asFloat();

    float clip_x1 = rect_t::get_x1(clip);
    float clip_y1 = rect_t::get_y1(clip);
    float clip_x2 = rect_t::get_x2(clip);
    float clip_y2 = rect_t::get_y2(clip);

    float dest_x2, dest_y2;

    if (destination.length() > 2) {
        dest_x2 = destination[2]->asFloat();
        dest_y2 = destination[3]->asFloat();
    } else {
        dest_x2 = dest_x + clip_x2 - clip_x1;
        dest_y2 = dest_y + clip_y2 - clip_y1;
    }

    float tex_x1 = clip_x1 / image_t::get_width(image);
    float tex_x2 = clip_x2 / image_t::get_width(image);
    float tex_y1 = clip_y1 / image_t::get_height(image);
    float tex_y2 = clip_y2 / image_t::get_height(image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glBindTexture(GL_TEXTURE_2D, image_t::get_texid(image));
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

    gl_check_error(cxt, caller);
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
