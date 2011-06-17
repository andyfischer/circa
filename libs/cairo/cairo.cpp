// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <cairo/cairo.h>

#include "types/color.h"

using namespace circa;

namespace cairo_support {

Type g_cairoContext_t;
Type g_cairoSurface_t;

cairo_t* as_cairo_context(TaggedValue* value)
{
    return (cairo_t*) get_pointer(value, &g_cairoContext_t);
}

cairo_surface_t* as_cairo_surface(TaggedValue* value)
{
    return (cairo_surface_t*) get_pointer(value, &g_cairoSurface_t);
}

void cairoContext_copy(Type*, TaggedValue* source, TaggedValue* dest)
{
    cairo_t* context = as_cairo_context(source);
    cairo_reference(context);
    set_pointer(dest, &g_cairoContext_t, context);
}
void cairoContext_release(Type*, TaggedValue* value)
{
    cairo_t* context = as_cairo_context(value);
    cairo_destroy(context);
}
void cairoSurface_copy(Type*, TaggedValue* source, TaggedValue* dest)
{
    cairo_surface_t* surface = as_cairo_surface(source);
    cairo_surface_reference(surface);
    set_pointer(dest, &g_cairoSurface_t, surface);
}
void cairoSurface_release(Type*, TaggedValue* value)
{
    cairo_surface_t* surface = as_cairo_surface(value);
    cairo_surface_destroy(surface);
}

float radians_to_degrees(float radians) { return radians * 180.0 / M_PI; }
float degrees_to_radians(float unit) { return unit * M_PI / 180.0; }

CA_FUNCTION(create_context_for_surface)
{
    cairo_surface_t* surface = as_cairo_surface(INPUT(0));
    set_pointer(OUTPUT, &g_cairoContext_t, cairo_create(surface));
}

CA_FUNCTION(create_image_surface)
{
    // user can't currently specify the format
    TaggedValue* size = INPUT(0);
    int width = as_int(list_get_index(size, 0));
    int height = as_int(list_get_index(size, 1));

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    set_pointer(OUTPUT, &g_cairoSurface_t, surface);
}

CA_FUNCTION(stroke)
{
    cairo_t* context = as_cairo_context(INPUT(0));
    cairo_stroke(context);
    set_null(OUTPUT);
}
CA_FUNCTION(paint)
{
    cairo_t* context = as_cairo_context(INPUT(0));
    cairo_stroke(context);
    set_null(OUTPUT);
}
CA_FUNCTION(set_source_color)
{
    cairo_t* context = as_cairo_context(INPUT(0));
    float r(0), g(0), b(0), a(0);
    get_color(INPUT(1), &r, &g, &b, &a);
    cairo_set_source_rgba(context, r, g, b, a);
    set_null(OUTPUT);
}
CA_FUNCTION(fill_preserve)
{
    cairo_t* context = as_cairo_context(INPUT(0));
    cairo_fill_preserve(context);
}
CA_FUNCTION(move_to)
{
    cairo_t* context = as_cairo_context(INPUT(0));
    float x(0), y(0);
    get_point(INPUT(1), &x, &y);
    cairo_move_to(context, x, y);
}
CA_FUNCTION(curve_to)
{
    cairo_t* context = as_cairo_context(INPUT(0));
    float x1(0), y1(0), x2(0), y2(0), x3(0), y3(0);
    get_point(INPUT(1), &x1, &x2);
    get_point(INPUT(2), &x1, &x2);
    get_point(INPUT(3), &x1, &x2);
    cairo_curve_to(context, x1, y1, x2, y2, x3, y3);
    set_null(OUTPUT);
}
CA_FUNCTION(line_to)
{
    cairo_t* context = as_cairo_context(INPUT(0));
    float x(0), y(0);
    get_point(INPUT(1), &x, &y);
    cairo_line_to(context, x, y);
    set_null(OUTPUT);
}
CA_FUNCTION(arc)
{
    cairo_t* context = as_cairo_context(INPUT(0));
    float center_x(0), center_y(0);
    get_point(INPUT(1), &center_x, &center_y);
    cairo_arc(context, center_x, center_y,
        FLOAT_INPUT(2),
        degrees_to_radians(FLOAT_INPUT(3)),
        degrees_to_radians(FLOAT_INPUT(4)));
}
CA_FUNCTION(new_sub_path)
{
    cairo_t* context = as_cairo_context(INPUT(0));
    cairo_new_sub_path(context);
}
CA_FUNCTION(close_path)
{
    cairo_t* context = as_cairo_context(INPUT(0));
    cairo_close_path(context);
}
CA_FUNCTION(set_line_width)
{
    cairo_t* context = as_cairo_context(INPUT(0));
    cairo_set_line_width(context, to_float(INPUT(1)));
    set_null(OUTPUT);
}
CA_FUNCTION(upload_surface_to_opengl)
{
    cairo_surface_t* surface = as_cairo_surface(INPUT(0));
    int texture_id = as_int(INPUT(1));

    unsigned char* pixels = cairo_image_surface_get_data(surface);
    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);
    int stride = cairo_image_surface_get_stride(surface);
    const int channels = 4;

    // Modify 'surface' so that colors are in RGBA order instead of BGRA
    for (int y=0; y < height; y++) for (int x=0; x < width; x++) {
        size_t offset = y * stride + x * channels;
        //unsigned char alpha = pixels[offset + 3];
        unsigned char b = pixels[offset + 0];
        unsigned char g = pixels[offset + 1];
        unsigned char r = pixels[offset + 2];
        unsigned char a = pixels[offset + 3];

        pixels[offset+0] = r;
        pixels[offset+1] = g;
        pixels[offset+2] = b;
        pixels[offset+3] = a;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
            width, height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    set_null(OUTPUT);
}

void setup(Branch& kernel)
{
    g_cairoContext_t.copy = cairoContext_copy;
    g_cairoContext_t.release = cairoContext_release;
    g_cairoSurface_t.copy = cairoSurface_copy;
    g_cairoSurface_t.release = cairoSurface_release;

    Branch& ns = nested_contents(kernel["cairo"]);

    install_type(ns["Surface"], &g_cairoSurface_t);
    install_type(ns["Context"], &g_cairoContext_t);

    install_function(ns["create_context_for_surface"], create_context_for_surface);
    install_function(ns["stroke"], stroke);
    install_function(ns["paint"], paint);
    install_function(ns["set_source_color"], set_source_color);
    install_function(ns["fill_preserve"], fill_preserve);
    install_function(ns["create_image_surface"], create_image_surface);
    install_function(ns["move_to"], move_to);
    install_function(ns["curve_to"], curve_to);
    install_function(ns["line_to"], line_to);
    install_function(ns["arc"], arc);
    install_function(ns["new_sub_path"], new_sub_path);
    install_function(ns["close_path"], close_path);
    install_function(ns["set_line_width"], set_line_width);
    install_function(ns["upload_surface_to_opengl"], upload_surface_to_opengl);
}

} // namespace cairo_support
