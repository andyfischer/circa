// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <SDL_image.h>

#include <importing_macros.h>
#include <circa.h>

#include "plastic_common_headers.h"

#include "image.h"
#include "sdl_util.h"
#include "gl_util.h"

using namespace circa;

namespace image_t {
    std::string get_filename(TaggedValue* v) { return v->getIndex(0)->asString(); }
    int get_texid(TaggedValue* v) { return v->getIndex(1)->asInt(); }
    int get_width(TaggedValue* v) { return v->getIndex(2)->asInt(); }
    int get_height(TaggedValue* v) { return v->getIndex(3)->asInt(); }

    void set_filename(TaggedValue* v, std::string const& s) { touch(v); make_string(v->getIndex(0), s); }
    void set_texid(TaggedValue* v, int id) { touch(v); set_int(v->getIndex(1), id); }
    void set_width(TaggedValue* v, int w) { touch(v); set_int(v->getIndex(2), w); }
    void set_height(TaggedValue* v, int h) { touch(v); set_int(v->getIndex(3), h); }
}

namespace rect_t {
    float get_x1(TaggedValue* v) { return v->getIndex(0)->toFloat(); }
    float get_y1(TaggedValue* v) { return v->getIndex(1)->toFloat(); }
    float get_x2(TaggedValue* v) { return v->getIndex(2)->toFloat(); }
    float get_y2(TaggedValue* v) { return v->getIndex(3)->toFloat(); }
}

GLuint load_image_to_texture(EvalContext* cxt, Term* caller, const char* filename)
{
    SDL_Surface* surface = IMG_Load(filename);
    if (surface == NULL) {
        std::stringstream msg;
        msg << "Error loading " << filename << ": " << SDL_GetError();
        error_occurred(cxt, caller, msg.str());
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

CA_FUNCTION(load_image)
{
    std::string const& filename = INPUT(0)->asString();

    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (surface == NULL) {
        error_occurred(CONTEXT, CALLER, "Error loading " + filename + ": " + SDL_GetError());
        return;
    }

    if (has_indexed_color(surface))
        surface = convert_indexed_color_to_true_color(surface);

    image_t::set_texid(OUTPUT, load_surface_to_texture(surface));
    image_t::set_filename(OUTPUT, filename);
    image_t::set_width(OUTPUT, surface->w);
    image_t::set_height(OUTPUT, surface->h);

    SDL_FreeSurface(surface);
}

CA_FUNCTION(draw_image)
{
    TaggedValue* image = INPUT(0);
    TaggedValue* point = INPUT(1);

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

    gl_check_error(CONTEXT, CALLER);
}

CA_FUNCTION(draw_image_clip)
{
#if 0
    TaggedValue* image = INPUT(0);
    TaggedValue* clip = INPUT(1);

    Branch& destination = INPUT(2)->asBranch();
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

    gl_check_error(CONTEXT, CALLER);
#endif
}

namespace image {

void setup(circa::Branch& branch)
{
    Branch& image_ns = branch["image"]->nestedContents;
    install_function(image_ns["_load"], load_image);
    install_function(image_ns["draw"], draw_image);
    install_function(image_ns["draw_clip_p"], draw_image_clip);
    install_function(image_ns["draw_clip_resized"], draw_image_clip);
}

} // namespace image
