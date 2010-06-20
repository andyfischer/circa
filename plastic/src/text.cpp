// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

#include <SDL_opengl.h>
#include <SDL_ttf.h>

#include "gl_util.h"
#include "image.h"
#include "sdl_util.h"
#include "text.h"

using namespace circa;

namespace text {

TypeRef TTF_Font_t;

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

SDL_Color unpack_sdl_color(TaggedValue* color)
{
    SDL_Color c = {};
    c.r = Uint8(color->getIndex(0)->asFloat() * 255.0);
    c.g = Uint8(color->getIndex(1)->asFloat() * 255.0);
    c.b = Uint8(color->getIndex(2)->asFloat() * 255.0);
    // c.a = colorTerm->getIndex(3)->asFloat() * 255.0;
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

struct RenderedText : TaggedValue
{
    int texid() { return getIndex(0)->asInt(); }
    int width() { return getIndex(1)->asInt(); }
    int height() { return getIndex(2)->asInt(); }
    TaggedValue* texidContainer() { return getIndex(0); }
    TaggedValue* widthContainer() { return getIndex(1); }
    TaggedValue* heightContainer() { return getIndex(2); }
    TaggedValue* color() { return getIndex(3); }
    std::string const& text() { return getIndex(4)->asString(); }
    TaggedValue* textContainer() { return getIndex(4); }
};

void render_text(EvalContext*, Term* caller)
{
    RenderedText* state = (RenderedText*) caller->input(0);
    touch(state);

    std::string const& inputText = as_string(caller->input(2));
    Term* inputColor = caller->input(3);
    bool changed_color = !state->color()->equals(inputColor);

    std::cout << "texid = " << state->texid() << std::endl;
    std::cout << "inputText = " << inputText << std::endl;
    std::cout << "state->text = " << state->text() << std::endl;

    if (state->texid() == 0 || state->text() != inputText || changed_color) {

        copy(caller->input(2), state->textContainer());

        // Clear results if text is empty
        if (inputText == "") {
            std::cout << "text is empty" << std::endl;
            set_int(state->texidContainer(), 0);
            set_int(state->widthContainer(), 0);
            set_int(state->heightContainer(), 0);
            copy(caller->input(0), caller);
            return;
        }

        // Render the text to a new surface, upload it as a texture, destroy the surface,
        // save the texture id.

        TTF_Font_ptr font = caller->input(1);

        SDL_Color sdlColor = unpack_sdl_color(caller->input(3));
        SDL_Surface *surface = TTF_RenderText_Blended(*font, inputText.c_str(), sdlColor);

        set_int(state->texidContainer(), load_surface_to_texture(surface));
        set_int(state->widthContainer(), surface->w);
        set_int(state->heightContainer(), surface->h);
        copy(inputColor, state->color());

        //SDL_SaveBMP(surface, "hello.bmp");

        SDL_FreeSurface(surface);
    }
    copy(state, caller);
}

void draw_rendered_text(EvalContext* cxt, Term* caller)
{
    RenderedText* obj = (RenderedText*) caller->input(0);

    if (obj->texid() == 0)
        return;

    int x = int(caller->input(1)->getIndex(0)->toFloat());
    int y = int(caller->input(1)->getIndex(1)->toFloat());

    glBindTexture(GL_TEXTURE_2D, obj->texid());
    glColor4f(1,1,1,1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);

    glTexCoord2d(0.0, 0.0);
    glVertex3i(x, y, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3i(x + obj->width(), y,0);
    glTexCoord2d(1.0, 1.0);
    glVertex3i(x + obj->width(), y + obj->height(),0);
    glTexCoord2d(0.0, 1.0);
    glVertex3i(x, y + obj->height(),0);

    glEnd();

    // Clean up state
    glBindTexture(GL_TEXTURE_2D, 0);

    gl_check_error(cxt, caller);
}

void pre_setup(Branch& branch)
{
    TTF_Font_t = Type::create();
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

    //TTF_Font_t = &as_type(branch["TTF_Font"]);
    //initialize_simple_pointer_type(TTF_Font_t);

    Branch& text_ns = branch["text"]->asBranch();

    install_function(text_ns["load_font"], load_font);
    install_function(text_ns["render_text"], render_text);
    install_function(text_ns["draw_rendered_text"], draw_rendered_text);
}

} // namespace text
