// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "Common.h"

#include "circa/circa.h"
#include "circa/objects.h"

#include "FontBitmap.h"
#include "RenderTarget.h"
#include "TextTexture.h"

#include "CircaBindings.h"

Color unpack_color(caValue* value)
{
    Color c;
    circa_vec4(value, &c.r, &c.g, &c.b, &c.a);
    return c;
}

caName get_tag(caValue* value)
{
    if (circa_is_name(value))
        return circa_name(value);
    else if (circa_is_list(value))
        return get_tag(circa_index(value, 0));
    else
        return 0;
}

void RenderTarget__getTextRender(caStack* stack)
{
    RenderTarget* target = (RenderTarget*) circa_get_pointer(circa_input(stack, 0));
    caValue* args = circa_input(stack, 1);
    caValue* cachedRender = target->getTextRender(args);
    circa_copy(cachedRender, circa_output(stack, 0));
}
void RenderTarget__sendCommand(caStack* stack)
{
    RenderTarget* target = (RenderTarget*) circa_get_pointer(circa_input(stack, 0));
    caValue* command = circa_input(stack, 1);
    target->sendCommand(command);
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

    circa_create_default_output(stack, 0)->value_data.asint = font;
}

static const caFunctionBinding g_imports[] = {
    {"RenderTarget.getTextRender", RenderTarget__getTextRender},
    {"RenderTarget.sendCommand", RenderTarget__sendCommand},
    {"RenderTarget.getViewportSize", RenderTarget__getViewportSize},
    {"create_font", create_font},
    {NULL, NULL}
};

void engine_bindings_install(caBranch* branch)
{
    circa_setup_pointer_type(circa_find_type(branch, "RenderTarget"));
    circa_setup_int_type(circa_find_type(branch, "Font"));
    circa_install_function_list(branch, g_imports);
}
