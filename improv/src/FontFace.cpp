
#include "FontFace.h"

bool g_ftLibraryInitialized;
FT_Library g_ftLibrary;

FontFace::FontFace()
{
    ftFace = NULL;
    cairoFace = NULL;
}

FontFace::~FontFace()
{
    cairo_font_face_destroy(cairoFace);
    FT_Done_Face(ftFace);
    circa_set_null(&rawData);
}

void FontFaceRelease(void* ptr)
{
    delete (FontFace*) ptr;
}

void load_font(caVM* vm)
{
    caValue* filename = circa_input(vm, 0);

    FontFace* font = new FontFace();

    circa_read_file_with_vm(vm, circa_string(filename), &font->rawData);
    if (circa_is_null(&font->rawData))
        return circa_output_error(vm, "Failed to load file");
    
    if (!g_ftLibraryInitialized) {
        FT_Init_FreeType(&g_ftLibrary);
        g_ftLibraryInitialized = true;
    }
    
    int error = FT_New_Memory_Face(g_ftLibrary,
                   (FT_Byte*) circa_blob(&font->rawData),
                   circa_blob_size(&font->rawData),
                   0,
                   &font->ftFace);

    if (error)
        return circa_output_error(vm, "FT_New_Memory_Face failed");

    font->cairoFace = cairo_ft_font_face_create_for_ft_face(font->ftFace, 0);

    circa_set_boxed_native_ptr(circa_output(vm), font, FontFaceRelease);
}

FontFace* as_font_face(caValue* value)
{
    return (FontFace*) circa_native_ptr(circa_index(value, 0));
}

void font_native_patch(caNativePatch* module)
{
    circa_patch_function(module, "load_font", load_font);
}
