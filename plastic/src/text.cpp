// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

#include <SDL_opengl.h>
#include <SDL_ttf.h>

#include "gl_util.h"
#include "image.h"
#include "text.h"

using namespace circa;

namespace text {

Type *TTF_Font_t = NULL;

class TTF_Font_ptr
{
    TaggedValue* _value;

public:
    TTF_Font_ptr(TaggedValue* value) { _value = value; }

    operator TTF_Font*() { return (TTF_Font*) get_pointer(_value, TTF_Font_t); }
    TTF_Font_ptr& operator=(TTF_Font* rhs) { set_pointer(_value, TTF_Font_t, rhs); return *this; }
    TTF_Font* operator*() { return (TTF_Font*) get_pointer(_value, TTF_Font_t); }
    TTF_Font* operator->() { return (TTF_Font*) get_pointer(_value, TTF_Font_t); }
};

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

void load_font(EvalContext* cxt, Term* term)
{
    TTF_Font_ptr state = term->input(0);
    TTF_Font_ptr output = term;

    if (*state != NULL) {
        output = state;
        return;
    }

    std::string path = as_string(term->input(1));
    int pointSize = term->input(2)->asInt();

    path = get_path_relative_to_source(term, path.c_str());

    TTF_Font* result = TTF_OpenFont(path.c_str(), pointSize);
    if (result == NULL) {
        std::stringstream err;
        err << "TTF_OpenFont failed to load " << path << " with error: " << TTF_GetError();
        error_occurred(cxt, term, err.str());
        return;
    }

    state = result;
    output = result;
}

struct RenderedText
{
    Term* _term;

    RenderedText(Term* term) : _term(term) {}

    Int texid() { return Int(_term->asBranch()[0]); }
    Int width() { return Int(_term->asBranch()[1]); }
    Int height() { return Int(_term->asBranch()[2]); }
    Term* color() { return _term->asBranch()[3]; }
    std::string const& text() { return as_string(_term->asBranch()[4]); }
    void set_text(const char* s) { set_str(_term->asBranch()[4], s); }
};

void render_text(EvalContext*, Term* caller)
{
    RenderedText state(caller->input(0));
    std::string const& text = as_string(caller->input(2));
    Term* color = caller->input(3);

    bool changed_color = !branch_t::equals(state.color(), color);

    if (state.texid() == 0 || state.text() != text || changed_color) {

        state.set_text(text.c_str());

        // Clear results if text is empty
        if (text == "") {
            state.texid() = 0;
            state.width() = 0;
            state.height() = 0;
            copy(caller->input(0), caller);
            return;
        }

        // Render the text to a new surface, upload it as a texture, destroy the surface,
        // save the texture id.

        TTF_Font_ptr font = caller->input(1);

        SDL_Color sdlColor = unpack_sdl_color(caller->input(3));
        SDL_Surface *surface = TTF_RenderText_Blended(*font, text.c_str(), sdlColor);

        state.texid() = load_surface_to_texture(surface);
        state.width() = surface->w;
        state.height() = surface->h;
        copy(color, state.color());

        SDL_SaveBMP(surface, "hello.bmp");

        SDL_FreeSurface(surface);
    }
    copy(caller->input(0), caller);
}

void draw_rendered_text(EvalContext* cxt, Term* caller)
{
    RenderedText output(caller->input(0));

    if (output.texid() == 0)
        return;

    int x = int(caller->input(1)->asBranch()[0]->toFloat());
    int y = int(caller->input(1)->asBranch()[1]->toFloat());

    glBindTexture(GL_TEXTURE_2D, output.texid());
    glColor4f(1,1,1,1);

    glBegin(GL_QUADS);

    glTexCoord2d(0.0, 0.0);
    glVertex3i(x, y, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3i(x + output.width(), y,0);
    glTexCoord2d(1.0, 1.0);
    glVertex3i(x + output.width(), y + output.height(),0);
    glTexCoord2d(0.0, 1.0);
    glVertex3i(x, y + output.height(),0);

    glEnd();

    // reset state
    glBindTexture(GL_TEXTURE_2D, 0);

    gl_check_error(cxt, caller);
}

void pre_setup(Branch& branch)
{
    TTF_Font_t = new Type();
    initialize_simple_pointer_type(TTF_Font_t);
    TTF_Font_t->name = "TTF_Font";

    import_type(branch, TTF_Font_t);
}

void setup(Branch& branch)
{
    if (TTF_Init() == -1) {
        std::cout << "TTF_Init failed with error: " << TTF_GetError();
        return;
    }

    Branch& text_ns = branch["text"]->asBranch();

    install_function(text_ns["load_font"], load_font);
    install_function(text_ns["render_text"], render_text);
    install_function(text_ns["draw_rendered_text"], draw_rendered_text);
}

} // namespace text
