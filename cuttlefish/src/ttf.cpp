// Copyright 2009 Andrew Fischer

#include "circa.h"

#include <SDL_opengl.h>
#include <SDL_ttf.h>

#include "textures.h"
#include "ttf.h"

using namespace circa;

namespace ttf {

SDL_Color unpack_sdl_color(int color)
{
    SDL_Color c;
    c.r = ((color & 0xff000000) >> 24);
    c.g = ((color & 0x00ff0000) >> 16);
    c.b = ((color & 0x0000ff00) >> 8);
    return c;
}

void load_font(Term* term)
{
    if (as<TTF_Font*>(term->input(0)) != NULL) {
        as<TTF_Font*>(term) = as<TTF_Font*>(term->input(0));
        return;
    }

    std::string path = term->input(1)->asString();
    int pointSize = term->input(2)->asInt();

    path = get_source_file_location(*term->owningBranch) + "/" + path;

    TTF_Font* result = TTF_OpenFont(path.c_str(), pointSize);
    if (result == NULL) {
        std::stringstream err;
        err << "TTF_OpenFont failed to load " << path << " with error: " << TTF_GetError();
        error_occurred(term, err.str());
        return;
    }

    as<TTF_Font*>(term->input(0)) = result;
    as<TTF_Font*>(term) = result;
}

struct RenderedText
{
    Term* _term;

    RenderedText(Term* term) : _term(term) {}

    int& texid() { return _term->field(0)->asInt(); }
    float& width() { return _term->field(1)->asFloat(); }
    float& height() { return _term->field(2)->asFloat(); }
};
    
void draw_text(Term* caller)
{
    RenderedText output(caller);
    float x = caller->input(2)->toFloat();
    float y = caller->input(3)->toFloat();

    if (output.texid() == 0) {
        // Render the text to a new surface, upload it as a texture, destroy the surface,
        // save the texture id.

        TTF_Font* font = as<TTF_Font*>(caller->input(0));
        std::string text = caller->input(1)->asString();
        SDL_Color color = {-1,-1,-1, -1}; // todo
        //SDL_Color bgcolor = {0, 0, 0, 0}; // todo

        SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), color);

        output.texid() = load_surface_to_texture(surface);
        output.width() = surface->w;
        output.height() = surface->h;

        SDL_FreeSurface(surface);
    }

    glBindTexture(GL_TEXTURE_2D, output.texid());

    glBegin(GL_QUADS);

    glTexCoord2d(0.0, 0.0);
    glVertex3f(x, y, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(x + output.width(), y,0);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(x + output.width(), y + output.height(),0);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(x, y + output.height(),0);

    glEnd();
}

void render_text(Term* caller)
{
    RenderedText output(caller);

    if (output.texid() == 0) {
        // Render the text to a new surface, upload it as a texture, destroy the surface,
        // save the texture id.

        TTF_Font* font = as<TTF_Font*>(caller->input(0));
        std::string text = caller->input(1)->asString();
        SDL_Color color = unpack_sdl_color(caller->input(2)->asInt());
        //SDL_Color bgcolor = {0, 0, 0, 0}; // todo

        SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), color);

        output.texid() = load_surface_to_texture(surface);
        output.width() = surface->w;
        output.height() = surface->h;

        SDL_FreeSurface(surface);
    }
}

// This should be turned into a member function:
void draw_rendered_text(Term* caller)
{
    RenderedText output(caller->input(0));
    float x = caller->input(1)->toFloat();
    float y = caller->input(2)->toFloat();

    glBindTexture(GL_TEXTURE_2D, output.texid());
    glColor4f(1,1,1,1);

    glBegin(GL_QUADS);

    glTexCoord2d(0.0, 0.0);
    glVertex3f(x, y, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(x + output.width(), y,0);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(x + output.width(), y + output.height(),0);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(x, y + output.height(),0);

    glEnd();
}

void initialize(circa::Branch& branch)
{
    if (TTF_Init() == -1) {
        std::cout << "TTF_Init failed with error: " << TTF_GetError();
        return;
    }

    import_type<TTF_Font*>(branch, "TTF_Font");

    import_function(branch, load_font, "load_font(state TTF_Font, string, int) : TTF_Font");
    branch.eval("type RenderedText { int texid, float width, float height }");
    import_function(branch, draw_text,
        "draw_text(TTF_Font, string, float x, float y, int) : RenderedText");

    import_function(branch, render_text, "render_text(TTF_Font, string, int color) : RenderedText");
    import_function(branch, draw_rendered_text,
        "draw_rendered_text(RenderedText, float x, float y)");
}

} // namespace ttf
