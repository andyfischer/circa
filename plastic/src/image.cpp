// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved

#include <circa.h>

#include "plastic.h"

using namespace circa;

namespace image {

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

void setup(circa::Branch& branch)
{
    Branch& image_ns = branch["image"]->asBranch();
    install_function(image_ns["_load"], load_image);
    install_function(image_ns["draw"], draw_image);
    install_function(function_t::get_previous_overload(image_ns["draw_clip"]), draw_image_clip);
    install_function(image_ns["draw_clip"], draw_image_clip);
}

} // namespace image
