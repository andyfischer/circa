// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "Common.h"

#include "circa/circa.h"
#include "circa/objects.h"

#include "FontBitmap.h"
#include "RenderCommand.h"
#include "RenderTarget.h"
#include "TextTexture.h"
#include "TextVbo.h"

struct RenderEntityRef
{
    RenderEntity* entity;
};

struct FontRef
{
    int font_id;
};

void RenderEntityRelease(void* obj)
{
    RenderEntityRef* ref = (RenderEntityRef*) obj;
    ref->entity->destroy();
}

Color unpack_color(caValue* value)
{
    Color c;
    circa_vec4(value, &c.r, &c.g, &c.b, &c.a);
    return c;
}

void RenderTarget__getFontRender(caStack* stack)
{
    RenderTarget* target = (RenderTarget*) circa_get_pointer(circa_input(stack, 0));
    caValue* args = circa_input(stack, 1);
    // output {int id, Point size}
}
void RenderTarget__sendCommand(caStack* stack)
{
    RenderTarget* target = (RenderTarget*) circa_get_pointer(circa_input(stack, 0));
    caValue* command = circa_input(stack, 1);
}
void RenderTarget__getViewportSize(caStack* stack)
{
    RenderTarget* target = (RenderTarget*) circa_get_pointer(circa_input(stack, 0));
    circa_set_vec2(circa_output(stack, 0), target->viewportWidth, target->viewportHeight);
}

void create_font(caStack* stack)
{
    const char* name = circa_string(circa_input(stack, 0));
    float size = circa_float(circa_input(stack, 1));
    int font = font_load(name, size);

    FontRef* ref = (FontRef*) circa_create_object_output(stack, 0);
    ref->font_id = font;
}

#if 0
void create_text_sprite(caStack* stack)
{
    RenderTarget* target = (RenderTarget*) circa_get_pointer(circa_input(stack, 0));

    RenderEntityRef* ref = (RenderEntityRef*) circa_create_object_output(stack, 0);
    ref->entity = TextTexture::create(target);
}

void TextTexture__setText(caStack* stack)
{
    RenderEntityRef* ref = (RenderEntityRef*) circa_object_input(stack, 0);
    TextTexture* sprite = (TextTexture*) ref->entity;
    const char* str = circa_string(circa_input(stack, 1));
    sprite->setText(str);
}
void TextTexture__setFont(caStack* stack)
{
    RenderEntityRef* ref = (RenderEntityRef*) circa_object_input(stack, 0);
    TextTexture* sprite = (TextTexture*) ref->entity;
    FontRef* fontRef = (FontRef*) circa_object_input(stack, 1);
    sprite->setFont(fontRef->font_id);
}
void TextTexture__setPosition(caStack* stack)
{
    RenderEntityRef* ref = (RenderEntityRef*) circa_object_input(stack, 0);
    TextTexture* sprite = (TextTexture*) ref->entity;
    float x,y;
    circa_vec2(circa_input(stack, 1), &x, &y);
    sprite->setPosition(x, y);
}
void TextTexture__setColor(caStack* stack)
{
    RenderEntityRef* ref = (RenderEntityRef*) circa_object_input(stack, 0);
    TextTexture* sprite = (TextTexture*) ref->entity;
    sprite->setColor(unpack_color(circa_input(stack, 1)));
}

void TextTexture__getRect(caStack* stack)
{
    RenderEntityRef* ref = (RenderEntityRef*) circa_object_input(stack, 0);
    TextTexture* sprite = (TextTexture*) ref->entity;

    // Save the rectangle in the result.
    FontBitmap* metrics = sprite->getMetrics();

    circa_set_vec4(circa_output(stack, 0),
            sprite->textVbo->posX,
            sprite->textVbo->posY,
            sprite->textVbo->posX + metrics->textWidth,
            sprite->textVbo->posY + metrics->descent + metrics->ascent);
}
#endif

static const caFunctionBinding g_imports[] = {
    {"create_font", create_font},
#if 0
    {"create_text_sprite", create_text_sprite},
    {"TextTexture.setText", TextTexture__setText},
    {"TextTexture.setFont", TextTexture__setFont},
    {"TextTexture.setPosition", TextTexture__setPosition},
    {"TextTexture.setColor", TextTexture__setColor},
    {"TextTexture.getRect", TextTexture__getRect},
#endif
    {NULL, NULL}
};

void engine_bindings_install(caBranch* branch)
{
    circa_setup_pointer_type(circa_find_type(branch, "RenderTarget"));
    circa_setup_object_type(circa_find_type(branch, "Font"),
                sizeof(FontRef), NULL);
    circa_setup_object_type(circa_find_type(branch, "TextTexture"),
                sizeof(RenderEntityRef), RenderEntityRelease);
    circa_setup_object_type(circa_find_type(branch, "LineList"),
                sizeof(RenderEntityRef), RenderEntityRelease);
    circa_install_function_list(branch, g_imports);
}
