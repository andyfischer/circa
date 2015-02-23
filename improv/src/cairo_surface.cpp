// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "improv_common.h"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <cstring>

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#include "circa/circa.h"
#include "FontFace.h"

cairo_t* as_cairo_context(caValue* value)
{
    return (cairo_t*) circa_native_ptr(circa_index(value, 0));
}

cairo_surface_t* as_cairo_surface(caValue* value)
{
    return (cairo_surface_t*) circa_native_ptr(circa_index(value, 0));
}

void ContextRelease(void* ptr)
{
    cairo_t* context = (cairo_t*) ptr;
    cairo_destroy(context);
}

void set_cairo_context(caValue* value, cairo_t* context)
{
    circa_set_boxed_native_ptr(value, context, ContextRelease);
}

void SurfaceRelease(void* ptr)
{
    cairo_surface_t* surface = (cairo_surface_t*) ptr;
    cairo_surface_destroy(surface);
}

// Convert from radians (used by Cairo) to degrees (used by Circa).
float radians_to_degrees(float radians) { return radians * 180.0 / M_PI; }
float degrees_to_radians(float degrees) { return degrees * M_PI / 180.0; }

void check_cairo_error(caVM* vm, cairo_t* context)
{
    cairo_status_t status = cairo_status(context);
    if (status != CAIRO_STATUS_SUCCESS) {
        circa_output_error(vm, cairo_status_to_string(status));
    }
}

void start_drawing(caVM* vm)
{
    cairo_surface_t* surface = as_cairo_surface(circa_input(vm, 0));
    cairo_t* context = cairo_create(surface);
    circa_set_boxed_native_ptr(circa_output(vm), context, ContextRelease);
}

void Canvas_save(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_save(context);
}
void Canvas_restore(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_restore(context);
    check_cairo_error(vm, context);
}

void Canvas_stroke(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_stroke(context);
    check_cairo_error(vm, context);
}
void Canvas_fill(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_fill(context);
    check_cairo_error(vm, context);
}
void Canvas_paint(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_paint(context);
    check_cairo_error(vm, context);
}
void Canvas_clip(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_clip(context);
    check_cairo_error(vm, context);
}
void Canvas_clip_preserve(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_clip_preserve(context);
    circa_set_null(circa_output(vm));
    check_cairo_error(vm, context);
}
void Canvas_reset_clip(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_reset_clip(context);
    circa_set_null(circa_output(vm));
}
void Canvas_set_source_color(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    float r(0), g(0), b(0), a(0);
    circa_vec4(circa_input(vm, 1), &r, &g, &b, &a);
    cairo_set_source_rgba(context, r, g, b, a);
    circa_set_null(circa_output(vm));
    check_cairo_error(vm, context);
}
void Canvas_fill_preserve(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_fill_preserve(context);
}
void Canvas_set_operator(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_set_operator(context, (cairo_operator_t) circa_int_input(vm, 1));
}

void Canvas__scale(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    float scaleX, scaleY;
    circa_vec2(circa_input(vm, 1), &scaleX, &scaleY);
    cairo_scale(context, scaleX, scaleY);
}

void Canvas__translate(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    float transformX, transformY;
    circa_vec2(circa_input(vm, 1), &transformX, &transformY);
    cairo_translate(context, transformX, transformY);
}

void Canvas_set_font_size(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    float size = circa_float_input(vm, 1);
    cairo_set_font_size(context, size);
    check_cairo_error(vm, context);
}
void Canvas_set_font_face(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    FontFace* font = as_font_face(circa_input(vm, 1));
    cairo_set_font_face(context, font->cairoFace);
    check_cairo_error(vm, context);
}
void Canvas_show_text(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    const char* str = circa_string_input(vm, 1);
    cairo_show_text(context, str);
    check_cairo_error(vm, context);
}
void Canvas_text_extents(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_text_extents_t extents;
    cairo_text_extents(context, circa_string_input(vm, 1), &extents);

    caValue* out = circa_output(vm);
    circa_set_list(out, 3);
    circa_set_vec2(circa_index(out, 0), extents.x_bearing, extents.y_bearing);
    circa_set_vec2(circa_index(out, 1), extents.width, extents.height);
    circa_set_vec2(circa_index(out, 2), extents.x_advance, extents.y_advance);
    check_cairo_error(vm, context);
}
void Canvas_font_extents(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_font_extents_t extents;
    cairo_font_extents(context, &extents);

    caValue* out = circa_output(vm);
    circa_set_list(out, 4);
    circa_set_float(circa_index(out, 0), extents.ascent);
    circa_set_float(circa_index(out, 1), extents.descent);
    circa_set_float(circa_index(out, 2), extents.height);
    circa_set_vec2(circa_index(out, 3), extents.max_x_advance, extents.max_y_advance);
    check_cairo_error(vm, context);
}

void Canvas_get_current_point(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    double x,y;
    cairo_get_current_point(context, &x, &y);
    circa_set_vec2(circa_output(vm), x, y);
    check_cairo_error(vm, context);
}

void Canvas_curve_to(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    float x1(0), y1(0), x2(0), y2(0), x3(0), y3(0);
    circa_vec2(circa_input(vm, 1), &x1, &x2);
    circa_vec2(circa_input(vm, 2), &x1, &x2);
    circa_vec2(circa_input(vm, 3), &x1, &x2);
    cairo_curve_to(context, x1, y1, x2, y2, x3, y3);
    circa_set_null(circa_output(vm));
}
void Canvas_move_to(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    float x(0), y(0);
    circa_vec2(circa_input(vm, 1), &x, &y);
    cairo_move_to(context, x, y);
}
void Canvas_rel_move_to(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    float x(0), y(0);
    circa_vec2(circa_input(vm, 1), &x, &y);
    cairo_rel_move_to(context, x, y);
}
void Canvas_rectangle(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    float x1(0), y1(0), x2(0), y2(0);
    circa_vec4(circa_input(vm, 1), &x1, &y1, &x2, &y2);
    cairo_rectangle(context, x1, y1, x2-x1, y2-y1);
    check_cairo_error(vm, context);
}
void Canvas_line_to(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    float x(0), y(0);
    circa_vec2(circa_input(vm, 1), &x, &y);
    cairo_line_to(context, x, y);
    check_cairo_error(vm, context);
}
void Canvas_rel_line_to(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    float x(0), y(0);
    circa_vec2(circa_input(vm, 1), &x, &y);
    cairo_rel_line_to(context, x, y);
}
void Canvas_arc(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    float center_x(0), center_y(0);
    circa_vec2(circa_input(vm, 1), &center_x, &center_y);
    cairo_arc(context, center_x, center_y,
        circa_float_input(vm, 2),
        degrees_to_radians(circa_float_input(vm, 3)),
        degrees_to_radians(circa_float_input(vm, 4)));
}
void Canvas_arc_negative(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    float center_x(0), center_y(0);
    circa_vec2(circa_input(vm, 1), &center_x, &center_y);
    cairo_arc_negative(context, center_x, center_y,
        circa_float_input(vm, 2),
        degrees_to_radians(circa_float_input(vm, 3)),
        degrees_to_radians(circa_float_input(vm, 4)));
}
void Canvas_new_sub_path(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_new_sub_path(context);
}
void Canvas_close_path(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_close_path(context);
}
void Canvas_set_line_width(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_set_line_width(context, circa_to_float(circa_input(vm, 1)));
    circa_set_null(circa_output(vm));
}
void Canvas_set_source_surface(caVM* vm)
{
    cairo_t* context = as_cairo_context(circa_input(vm, 0));
    cairo_surface_t* surface = as_cairo_surface(circa_input(vm, 1));
    float originX(0), originY(0);
    circa_vec2(circa_input(vm, 2), &originX, &originY);
    cairo_set_source_surface(context, surface, originX, originY);

    check_cairo_error(vm, context);
}

void make_image_surface(caVM* vm)
{
    float width, height;
    circa_vec2(circa_input(vm, 0), &width, &height);

    // user can't currently specify the format
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
        int(width), int(height));

    cairo_status_t status = cairo_surface_status(surface);
    if (status != CAIRO_STATUS_SUCCESS) {
        circa_output_error(vm, cairo_status_to_string(status));
        return;
    }

    circa_set_boxed_native_ptr(circa_output(vm), surface, SurfaceRelease);
}

void load_image(caVM* vm)
{
#if 0
    struct PngReadContext {
        int readPos;
        circa::Value contents;

        PngReadContext() : readPos(0) {}

        static cairo_status_t readFunc(void* cxt, unsigned char* data, unsigned int length)
        {
            PngReadContext* context = (PngReadContext*) cxt;

            if (context->readPos + length > circa_blob_length(&context->contents))
                return CAIRO_STATUS_READ_ERROR;

            memcpy(data, circa_blob(&context->contents) + context->readPos, length);
            context->readPos += length;
            return CAIRO_STATUS_SUCCESS;
        }
    };

    const char* filename = circa_string_input(vm, 0);

    PngReadContext pngReadContext;
    circa_read_file_with_stack(vm, filename, &pngReadContext.contents);

    if (circa_is_null(&pngReadContext.contents))
        return circa_output_error(vm, "Failed to load file");
    
    cairo_surface_t* surface = cairo_image_surface_create_from_png_stream(
        PngReadContext::readFunc, &pngReadContext);

    cairo_status_t status = cairo_surface_status(surface);
    if (status != CAIRO_STATUS_SUCCESS)
        return circa_output_error(vm, cairo_status_to_string(status));

    caValue* out = circa_set_default_output(vm, 0);
    circa_set_native_ptr(circa_index(out, 0), surface, SurfaceRelease);
#endif
}

void Surface__write_to_png(caVM* vm)
{
    cairo_surface_t* surface = as_cairo_surface(circa_input(vm, 0));
    const char* filename = circa_string_input(vm, 1);

    cairo_status_t status = cairo_surface_write_to_png(surface, filename);

    if (status != CAIRO_STATUS_SUCCESS) {
        circa_output_error(vm, "cairo_surface_write_to_png failed");
    }
}

void Surface__size(caVM* vm)
{
    cairo_surface_t* surface = as_cairo_surface(circa_input(vm, 0));
    double x1, x2, y1, y2;
    cairo_t* cr = cairo_create(surface);
    cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
    cairo_destroy(cr);

    circa_set_vec2(circa_output(vm), x2 - x1, y2 - y1);
}

void Surface__image_blob(caVM* vm)
{
    caValue* surfaceVal = circa_input(vm, 0);
    cairo_surface_t* surface = as_cairo_surface(surfaceVal);
    cairo_surface_flush(surface);

    circa_set_blob_from_backing_value(
        circa_output(vm),
        surfaceVal,
        (char*) cairo_image_surface_get_data(surface),
        cairo_image_surface_get_width(surface) * cairo_image_surface_get_height(surface));
}

void cairo_native_patch(caNativePatch* module)
{
    circa_patch_function(module, "Canvas.save", Canvas_save);
    circa_patch_function(module, "Canvas.restore", Canvas_restore);
    circa_patch_function(module, "Canvas.stroke", Canvas_stroke);
    circa_patch_function(module, "Canvas.fill", Canvas_fill);
    circa_patch_function(module, "Canvas.paint", Canvas_paint);
    circa_patch_function(module, "Canvas.clip", Canvas_clip);
    circa_patch_function(module, "Canvas.clip_preserve", Canvas_clip_preserve);
    circa_patch_function(module, "Canvas.reset_clip", Canvas_reset_clip);
    circa_patch_function(module, "Canvas.set_source_color", Canvas_set_source_color);
    circa_patch_function(module, "Canvas.fill_preserve", Canvas_fill_preserve);
    circa_patch_function(module, "Canvas.set_operator", Canvas_set_operator);
    circa_patch_function(module, "Canvas.scale", Canvas__scale);
    circa_patch_function(module, "Canvas.translate", Canvas__translate);
    circa_patch_function(module, "Canvas.set_font_size", Canvas_set_font_size);
    circa_patch_function(module, "Canvas.set_font_face", Canvas_set_font_face);
    circa_patch_function(module, "Canvas.show_text", Canvas_show_text);
    circa_patch_function(module, "Canvas.text_extents", Canvas_text_extents);
    circa_patch_function(module, "Canvas.font_extents", Canvas_font_extents);
    circa_patch_function(module, "Canvas.get_current_point", Canvas_get_current_point);
    circa_patch_function(module, "Canvas.curve_to", Canvas_curve_to);
    circa_patch_function(module, "Canvas.move_to", Canvas_move_to);
    circa_patch_function(module, "Canvas.rel_move_to", Canvas_rel_move_to);
    circa_patch_function(module, "Canvas.rectangle", Canvas_rectangle);
    circa_patch_function(module, "Canvas.line_to", Canvas_line_to);
    circa_patch_function(module, "Canvas.rel_line_to", Canvas_rel_line_to);
    circa_patch_function(module, "Canvas.arc", Canvas_arc);
    circa_patch_function(module, "Canvas.arc_negative", Canvas_arc_negative);
    circa_patch_function(module, "Canvas.new_sub_path", Canvas_new_sub_path);
    circa_patch_function(module, "Canvas.close_path", Canvas_close_path);
    circa_patch_function(module, "Canvas.set_line_width", Canvas_set_line_width);
    circa_patch_function(module, "Canvas.set_source_surface", Canvas_set_source_surface);
    circa_patch_function(module, "make_image_surface", make_image_surface);
    circa_patch_function(module, "load_image", load_image);
    circa_patch_function(module, "Surface.write_to_png", Surface__write_to_png);
    circa_patch_function(module, "Surface.size", Surface__size);
    circa_patch_function(module, "Surface.image_blob", Surface__image_blob);
    circa_patch_function(module, "Surface.start_drawing", start_drawing);
    font_native_patch(module);
    circa_finish_native_patch(module);
}
