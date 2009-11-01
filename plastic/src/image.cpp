// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved

#include <SDL_image.h>

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

void load_image(Term* caller)
{
    std::string& filename = caller->input(0)->asString();
    Image result(caller);

    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (surface == NULL) {
        error_occurred(caller, "Error loading " + filename + ": " + SDL_GetError());
        return;
    }

    result.texid() = load_surface_to_texture(surface);
    result.width() = surface->w;
    result.height() = surface->h;
    result.filename() = filename;

    SDL_FreeSurface(surface);

    std::cout << "loaded " << filename << std::endl;
}

void draw_image_clipped(Term* caller)
{

}

void setup(circa::Branch& branch)
{
    Branch& image_ns = branch["image"]->asBranch();
    install_function(image_ns["_load_image"], load_image);
}

} // namespace image
