// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

#include <SDL_opengl.h>
#include <SDL_ttf.h>

#include "gl_common.h"
#include "sdl_utils.h"
#include "ttf.h"

using namespace circa;

namespace ttf {

SDL_Color unpack_sdl_color(Term* colorTerm)
{
    Branch& color = as_branch(colorTerm);
    SDL_Color c = {};
    c.r = Uint8(color[0]->asFloat() * 255.0);
    c.g = Uint8(color[1]->asFloat() * 255.0);
    c.b = Uint8(color[2]->asFloat() * 255.0);
    // c.a = color[3]->asFloat() * 255.0;
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

    path = get_path_relative_to_source(term, path);

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

    int& texid() { return _term->asBranch()[0]->asInt(); }
    float& width() { return _term->asBranch()[1]->asFloat(); }
    float& height() { return _term->asBranch()[2]->asFloat(); }
    Branch& color() { return _term->asBranch()[3]->asBranch(); }
    std::string& text() { return _term->asBranch()[4]->asString(); }
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
        output.width() = float(surface->w);
        output.height() = float(surface->h);

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

    gl_check_error(caller);
}

void render_text(Term* caller)
{
    RenderedText state(caller->input(0));
    std::string text = caller->input(2)->asString();

    if (state.texid() == 0 || state.text() != text) {

        state.text() = text;

        // Clear results if text is empty
        if (text == "") {
            state.texid() = 0;
            state.width() = 0;
            state.height() = 0;
            return;
        }

        // Render the text to a new surface, upload it as a texture, destroy the surface,
        // save the texture id.

        TTF_Font* font = as<TTF_Font*>(caller->input(1));
        SDL_Color color = unpack_sdl_color(caller->input(3));
        //SDL_Color bgcolor = {0, 0, 0, 0}; // todo

        SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), color);

        state.texid() = load_surface_to_texture(surface);
        state.width() = float(surface->w);
        state.height() = float(surface->h);

        SDL_FreeSurface(surface);
    }
    assign_value(caller->input(0), caller);
}

void draw_rendered_text(Term* caller)
{
    RenderedText output(caller->input(0));

    if (output.texid() == 0)
        return;

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

    // reset state
    glBindTexture(GL_TEXTURE_2D, 0);

    gl_check_error(caller);
}

void setup(Branch& branch)
{
    if (TTF_Init() == -1) {
        std::cout << "TTF_Init failed with error: " << TTF_GetError();
        return;
    }

    import_type<TTF_Font*>(branch["TTF_Font"]);

    type_t::enable_default_value(branch["TTF_Font"]);
    as<TTF_Font*>(type_t::get_default_value(branch["TTF_Font"])) = 0;

    Branch& text_ns = branch["text"]->asBranch();

    install_function(text_ns["load_font"], load_font);
    //install_function(text_ns["draw_text"], draw_text);
    install_function(text_ns["render_text"], render_text);
    install_function(text_ns["draw_rendered_text"], draw_rendered_text);
}

} // namespace ttf
