// Copyright 2009 Paul Hodge

#include "circa.h"

#include <SDL_ttf.h>

#include "ttf.h"

using namespace circa;

namespace ttf {

Term* TTF_FONT_TYPE = NULL;

void load_font(Term* term)
{
    if (as<TTF_Font*>(term) != NULL)
        return;

    std::string path = term->input(0)->asString();
    int pointSize = term->input(1)->asInt();

    TTF_Font* result = TTF_OpenFont(path.c_str(), pointSize);
    if (result == NULL) {
        std::stringstream err;
        err << "TTF_OpenFont failed to load " << path << " with error: " << TTF_GetError();
        error_occurred(term, err.str());
        return;
    }

    as<TTF_Font*>(term) = result;
}

void render_text(Term* term)
{
    // Render the text to a new surface, upload it as a texture, destroy the surface
    // and return the texture id. All quite inefficient.

    //TTF_Font* font = as<TTF_Font*>(term->input(0));

    // TODO
}
    
void initialize(circa::Branch& branch)
{
    if (TTF_Init() == -1) {
        std::cout << "TTF_Init failed with error: " << TTF_GetError();
        return;
    }

    import_type<TTF_Font*>(branch, "TTF_Font");
    import_function(branch, load_font, "load_font(string, int) : TTF_Font");
    import_function(branch, load_font, "render_text(TTF_Font, string)");
}

} // namespace ttf
