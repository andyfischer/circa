// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "plastic_common_headers.h"

#include "circa.h"
#include "importing_macros.h"
#include "refcounted_type_wrapper.h"

#include <SDL_ttf.h>

#include "gl_util.h"
#include "image.h"
#include "sdl_util.h"
#include "text.h"

using namespace circa;

namespace text {

struct FontRef
{
    int _refCount;
    TTF_Font* ttfFont;

    FontRef() : ttfFont(NULL) {}

    static Type* singleton;
};

Type* FontRef::singleton;

struct Font : public circa::TaggedValue
{
    FontRef* contents()
    {
        return (FontRef*) get_pointer(this);
    }

    static Font* checkCast(TaggedValue* val)
    {
        if (val->value_type->name != "Font")
            return NULL;
        return (Font*) val;
    }

    static Font* lazyCast(TaggedValue* val)
    {
        if (val->value_type->name != "Font")
            change_type(val, FontRef::singleton);
        return (Font*) val;
    }

    static Font* cast(TaggedValue* val)
    {
        circa::reset(val);
        change_type(val, FontRef::singleton);
        return (Font*) val;
    }
};

SDL_Color unpack_sdl_color(TaggedValue* color)
{
    SDL_Color c = {};
    c.r = Uint8(color->getIndex(0)->asFloat() * 255.0);
    c.g = Uint8(color->getIndex(1)->asFloat() * 255.0);
    c.b = Uint8(color->getIndex(2)->asFloat() * 255.0);
    return c;
}

CA_FUNCTION(load_font)
{
    Font* output = Font::cast(OUTPUT);

    std::string path = STRING_INPUT(0);
    int pointSize = INT_INPUT(1);

    //std::cout << "Calling TTF_OpenFont(" << path.c_str() << std::endl;
    
    TTF_Font* result = TTF_OpenFont(path.c_str(), pointSize);
    if (result == NULL) {
        std::stringstream err;
        err << "TTF_OpenFont failed to load " << path << " with error: " << TTF_GetError();
        return error_occurred(CONTEXT, CALLER, err.str());
    }

    output->contents()->ttfFont = result;
}

struct RenderedText : public TaggedValue
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

    static RenderedText* cast(TaggedValue* val)
    {
        change_type(val, singleton);
        return (RenderedText*) val;
    }

    static Type* singleton;
};

Type* RenderedText::singleton;

CA_FUNCTION(render_text)
{
    std::string const& inputText = as_string(INPUT(1));
    TaggedValue* inputColor = INPUT(2);

    RenderedText* output = RenderedText::cast(OUTPUT);

    copy(INPUT(1), output->textContainer());

    // Clear results if text is empty
    if (inputText == "") {
        set_int(output->texidContainer(), 0);
        set_int(output->widthContainer(), 0);
        set_int(output->heightContainer(), 0);
        return;
    }

    // Render the text to a new surface, upload it as a texture, destroy the surface,
    // record the texture id.

    Font* font = Font::checkCast(INPUT(0));

    SDL_Color sdlColor = unpack_sdl_color(INPUT(2));
    SDL_Surface *surface = TTF_RenderText_Blended(font->contents()->ttfFont,
            inputText.c_str(), sdlColor);

    set_int(output->texidContainer(), load_surface_to_texture(surface));
    set_int(output->widthContainer(), surface->w);
    set_int(output->heightContainer(), surface->h);
    copy(inputColor, output->color());

    //SDL_SaveBMP(surface, "hello.bmp");

    SDL_FreeSurface(surface);
}

CA_FUNCTION(draw_rendered_text)
{
    RenderedText* obj = (RenderedText*) INPUT(0);

    if (obj->texid() == 0)
        return;

    int x = int(INPUT(1)->getIndex(0)->toFloat());
    int y = int(INPUT(1)->getIndex(1)->toFloat());

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

    gl_check_error(CONTEXT, CALLER);
}

CA_FUNCTION(get_metrics)
{
    Font* font = Font::checkCast(INPUT(0));
    const char* str = as_cstring(INPUT(1));

    List* output = List::cast(OUTPUT, 2);

    int w;
    int h;

    TTF_SizeText(font->contents()->ttfFont, str, &w, &h);

    set_int(output->get(0), w);
    set_int(output->get(1), h);
}

void setup(Branch& branch)
{
    FontRef::singleton = unbox_type(branch["Font"]);
    intrusive_refcounted::setup_type<FontRef>(FontRef::singleton);

    if (TTF_Init() == -1) {
        std::cout << "TTF_Init failed with error: " << TTF_GetError();
        return;
    }

    Branch& text_ns = branch["text"]->nestedContents;
    install_function(text_ns["load_font_internal"], load_font);
    install_function(text_ns["render_text_internal"], render_text);
    install_function(text_ns["draw_rendered_text"], draw_rendered_text);
    install_function(text_ns["get_metrics"], get_metrics);

    RenderedText::singleton = unbox_type(text_ns["RenderedText"]);
}

} // namespace text
