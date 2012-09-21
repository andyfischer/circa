// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <pango/pangocairo.h>

#include "circa/circa.h"


void FreeCairoSurface(void* obj)
{
    cairo_surface_destroy((CairoImageSurface*) obj);
}

void rendertext(cairo_t *cr)
{
	PangoLayout *layout;
	PangoFontDescription *desc;

	cairo_translate(cr, 10, 20);
	layout = pango_cairo_create_layout(cr);
        pango_layout_set_text(layout, "Hello World!", -1);
	desc = pango_font_description_from_string("Sans Bold 12");
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
	pango_cairo_update_layout(cr, layout);
	pango_cairo_show_layout(cr, layout);

	g_object_unref(layout);
}

int cairo_ui_test(int argc, char **argv)
{
    cairo_t *cr;
    char *filename;
    cairo_status_t status;
    cairo_surface_t *surface;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 300, 100);
	cr = cairo_create(surface);

	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_paint(cr);

    /*

    if (argc != 2)
    {
        g_printerr ("Usage: cairosimple OUTPUT_FILENAME\n");
        return 1;
    }
    filename = argv[1];
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                    2 * RADIUS, 2 * RADIUS);
    cr = cairo_create (surface);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    draw_text(cr);
    cairo_destroy(cr);
    status = cairo_surface_write_to_png (surface, filename);
    cairo_surface_destroy (surface);
    if (status != CAIRO_STATUS_SUCCESS)
    {
        g_printerr ("Could not save png to '%s'\n", filename);
        return 1;
    }
    */
    return 0;
}
