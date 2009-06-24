// Copyright 2009 Paul Hodge

#include "circa.h"

#include <SDL_opengl.h>
#include <SDL_ttf.h>

#include "textures.h"

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

void draw_text(Term* caller)
{
    Branch& output = as_branch(caller);
    int& texid = output[0]->asInt();
    float& width = output[1]->asFloat();
    float& height = output[2]->asFloat();
    float x = caller->input(2)->toFloat();
    float y = caller->input(3)->toFloat();

    if (texid == 0) {
        // Render the text to a new surface, upload it as a texture, destroy the surface,
        // create the geometry, save the texture id.

        TTF_Font* font = as<TTF_Font*>(caller->input(0));
        std::string text = caller->input(1)->asString();
        SDL_Color color = {-1,-1,-1, -1}; // todo
        SDL_Color bgcolor = {0, 0, 0, 0}; // todo

        SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), color);

        texid = load_surface_to_texture(surface);
        width = surface->w;
        height = surface->h;
    }

    glBindTexture(GL_TEXTURE_2D, texid);

    glBegin(GL_QUADS);

    glTexCoord2d(0.0, 0.0);
    glVertex3f(x, y, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(x + width, y,0);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(x + width, y + height,0);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(x, y + height,0);

    glEnd();
}
    
void initialize(circa::Branch& branch)
{
    if (TTF_Init() == -1) {
        std::cout << "TTF_Init failed with error: " << TTF_GetError();
        return;
    }

    import_type<TTF_Font*>(branch, "TTF_Font");
    import_function(branch, load_font, "load_font(string, int) : TTF_Font");
    branch.eval("type draw_text__output { int texid, float width, float height }");
    import_function(branch, draw_text,
        "draw_text(TTF_Font, string, float x, float y, int) : draw_text__output");
}

} // namespace ttf
