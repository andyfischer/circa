// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "Common.h"

#include "circa/circa.h"
#include "circa/objects.h"

#include "FontBitmap.h"
#include "RenderCommand.h"
#include "TextSprite.h"
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

void create_font(caStack* stack)
{
    const char* name = circa_string(circa_input(stack, 0));
    float size = circa_float(circa_input(stack, 1));
    int font = font_load(name, size);

    FontRef* ref = (FontRef*) circa_create_object_output(stack, 0);
    ref->font_id = font;
}

void create_text_sprite(caStack* stack)
{
    RenderTarget* target = (RenderTarget*) circa_get_pointer(circa_input(stack, 0));

    RenderEntityRef* ref = (RenderEntityRef*) circa_create_object_output(stack, 0);
    ref->entity = TextSprite::create(target);
}

void TextSprite__setText(caStack* stack)
{
    RenderEntityRef* ref = (RenderEntityRef*) circa_object_input(stack, 0);
    TextSprite* sprite = (TextSprite*) ref->entity;
    const char* str = circa_string(circa_input(stack, 1));
    sprite->setText(str);
}
void TextSprite__setFont(caStack* stack)
{
    RenderEntityRef* ref = (RenderEntityRef*) circa_object_input(stack, 0);
    TextSprite* sprite = (TextSprite*) ref->entity;
    FontRef* fontRef = (FontRef*) circa_object_input(stack, 1);
    sprite->setFont(fontRef->font_id);
}
void TextSprite__setPosition(caStack* stack)
{
    RenderEntityRef* ref = (RenderEntityRef*) circa_object_input(stack, 0);
    TextSprite* sprite = (TextSprite*) ref->entity;
    float x,y;
    circa_vec2(circa_input(stack, 1), &x, &y);
    sprite->setPosition(x, y);
}
void TextSprite__setColor(caStack* stack)
{
    RenderEntityRef* ref = (RenderEntityRef*) circa_object_input(stack, 0);
    TextSprite* sprite = (TextSprite*) ref->entity;
    sprite->setColor(unpack_color(circa_input(stack, 1)));
}

void TextSprite__getRect(caStack* stack)
{
    RenderEntityRef* ref = (RenderEntityRef*) circa_object_input(stack, 0);
    TextSprite* sprite = (TextSprite*) ref->entity;

    // Save the rectangle in the result.
    FontBitmap* metrics = sprite->getMetrics();

    circa_set_vec4(circa_output(stack, 0),
            sprite->textVbo->posX,
            sprite->textVbo->posY,
            sprite->textVbo->posX + metrics->textWidth,
            sprite->textVbo->posY + metrics->descent + metrics->ascent);
}

static const caFunctionBinding g_imports[] = {
    {"create_font", create_font},
    {"create_text_sprite", create_text_sprite},
    {"TextSprite.setText", TextSprite__setText},
    {"TextSprite.setFont", TextSprite__setFont},
    {"TextSprite.setPosition", TextSprite__setPosition},
    {"TextSprite.setColor", TextSprite__setColor},
    {"TextSprite.getRect", TextSprite__getRect},
    {NULL, NULL}
};

void engine_bindings_install(caBranch* branch)
{
    circa_setup_pointer_type(circa_find_type(branch, "RenderTarget"));
    circa_setup_object_type(circa_find_type(branch, "Font"),
                sizeof(FontRef), NULL);
    circa_setup_object_type(circa_find_type(branch, "TextSprite"),
                sizeof(RenderEntityRef), RenderEntityRelease);
    circa_setup_object_type(circa_find_type(branch, "LineList"),
                sizeof(RenderEntityRef), RenderEntityRelease);
    circa_install_function_list(branch, g_imports);
}
