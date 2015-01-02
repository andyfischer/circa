
#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#include "circa/circa.h"

struct FontFace {
    circa::Value rawData;
    FT_Face ftFace;
    cairo_font_face_t* cairoFace;

    FontFace();
    ~FontFace();
};


FontFace* as_font_face(caValue* value);

void font_native_patch(caNativePatch* module);
