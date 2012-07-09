// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "Common.h"

#include "circa/circa.h"

#include "FontBitmap.h"
#include "RenderCommand.h"
#include "TextSprite.h"

struct TextSpriteReference
{
};
void EntityRelease(caValue* value)
{
    ((RenderEntity*) circa_get_pointer(value))->destroy();
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

    caValue* out = circa_create_default_output(stack, 0);
    circa_set_int(circa_index(out, 0), font);
}

void create_text_sprite(caStack* stack)
{
    RenderTarget* target = (RenderTarget*) circa_get_pointer(circa_input(stack, 0));
    caValue* out = circa_create_default_output(stack, 0);
    circa_handle_set_object(out, TextSprite::create(target), EntityRelease);
}

void TextSprite__setText(caStack* stack)
{
    TextSprite* sprite = (TextSprite*) circa_handle_get_object(circa_input(stack, 0));
    const char* str = circa_string(circa_input(stack, 1));
    sprite->setText(str);
}
void TextSprite__setFont(caStack* stack)
{
    TextSprite* sprite = (TextSprite*) circa_handle_get_object(circa_input(stack, 0));
    int font = circa_int(circa_index(circa_input(stack, 1), 0));
    sprite->setFont(font);
}
void TextSprite__setPosition(caStack* stack)
{
    TextSprite* sprite = (TextSprite*) circa_handle_get_object(circa_input(stack, 0));
    float x,y;
    circa_vec2(circa_input(stack, 1), &x, &y);
    sprite->setPosition(x, y);
}
void TextSprite__setColor(caStack* stack)
{
    TextSprite* sprite = (TextSprite*) circa_handle_get_object(circa_input(stack, 0));
    sprite->setColor(unpack_color(circa_input(stack, 1)));
}

static const caFunctionBinding g_imports[] = {
    {"create_font", create_font},
    {"create_text_sprite", create_text_sprite},
    {"TextSprite.setText", TextSprite__setText},
    {"TextSprite.setFont", TextSprite__setFont},
    {"TextSprite.setPosition", TextSprite__setPosition},
    {"TextSprite.setColor", TextSprite__setColor},
    {NULL, NULL}
};

void engine_bindings_install(caBranch* branch)
{
    circa_install_function_list(branch, g_imports);
}
