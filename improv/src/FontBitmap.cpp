// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <string>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Common.h"
#include "FontBitmap.h"

FT_Library g_library = NULL;

// Font table
const int MAX_FONTS = 200;

struct FontFace
{
    FT_Face face;
};

int g_nextFontId = 1; // Deliberately skip font id 0
FontFace g_fontTable[MAX_FONTS];

struct FontCacheKey
{
    std::string filename;
    int size;

    bool operator<(FontCacheKey const& rhs) const
    {
        if (filename != rhs.filename)
            return filename < rhs.filename;
        else
            return size < rhs.size;
    }
};

std::map<FontCacheKey, FontFace*> g_fontCache;

FontFace* font_load(const char* filename, int pixelHeight)
{
    FontCacheKey cacheKey;
    cacheKey.filename = filename;
    cacheKey.size = pixelHeight;

    std::map<FontCacheKey, FontFace*>::const_iterator it = g_fontCache.find(cacheKey);
    if (it != g_fontCache.end())
        return it->second;

    FT_Error error;

    // Load the library if necessary
    if (g_library == NULL) {
        error = FT_Init_FreeType( &g_library );
        if (error) {
            Log("Failed to initialize FreeType");
            return NULL;
        }
        
        memset(g_fontTable, 0, sizeof(g_fontTable));
    }

    FT_Face face;
    error = FT_New_Face( g_library, filename, 0, &face );
    
    if ( error == FT_Err_Unknown_File_Format ) {
        Log("FreeType failed to load font file %s (unknown file format)", filename);
        return NULL;
    }
    else if ( error ) {
        Log("FreeType failed to load font file %s", filename);
        return NULL;
    }
    
    int fontId = g_nextFontId++;
    FontFace* fontFace = &g_fontTable[fontId];
    fontFace->face = face;
    
    FT_Set_Pixel_Sizes(face, 0, pixelHeight);

    printf("Loaded font from file %s, with id %d\n", filename, fontId);

    g_fontCache[cacheKey] = fontFace;
    
    return fontFace;
}

int font_get_face_height(FontFace* face)
{
    if (face == NULL)
        return 0;
    
    FT_Face ftface = face->face;
    return (ftface->ascender >> 6) + ((-ftface->descender) >> 6);
}

void load_glyph(FontBitmap* op, int charIndex)
{
    FT_UInt glyph_index;
    
    FT_Face face = op->face->face;
    
    glyph_index = FT_Get_Char_Index( face, op->str[charIndex] );
    
    // Load glyph image into the slot (erase previous one)
    FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT );
}

void font_update_metrics(FontBitmap* op)
{
    FT_Face face = op->face->face;

    if (face == NULL) {
        Log("font_update_metrics called with NULL font");
        return;
    }

    int overallWidth = 0;
    int highestBearingY = 0;
    int penX = 0;

    for (int i = 0 ; op->str[i] != 0; i++ ) {
        load_glyph(op, i);

        int left = penX + (face->glyph->metrics.horiBearingX >> 6);
        int right = left + (face->glyph->metrics.width >> 6);
        penX += face->glyph->advance.x >> 6;
        
        if (i == 0)
            op->originX = left;
        
        int bearingY = face->glyph->metrics.horiBearingY >> 6;
        if (bearingY > highestBearingY)
            highestBearingY = bearingY;

        overallWidth = right;
    }

    op->textWidth = overallWidth;
    
    // metrics.ascender seems to be way too high
    //op->ascent = face->size->metrics.ascender >> 6;
    op->ascent = highestBearingY;
    op->descent = face->size->metrics.descender >> 6;
    op->originY = highestBearingY;
    
    op->bitmapSizeX = overallWidth;
    op->bitmapSizeY = face->size->metrics.height >> 6;//(face->bbox.yMax - face->bbox.yMin) >> 6;
}

bool font_render(FontBitmap* op)
{
    if (op->face == NULL)
        return false;
    if (op->str == NULL)
        return false;
    
    FT_Face face = op->face->face;
    FT_Error error;

    int bitmapSize = op->bitmapSizeX * op->bitmapSizeY;
    op->bitmap = (char*) malloc(bitmapSize);
    
    memset(op->bitmap, 0, bitmapSize);

    // Pen will be used as the "origin" position.
    FT_Vector pen;
    pen.x = 0;
    pen.y = op->originY;
    
    for (int i = 0 ; op->str[i] != 0; i++ ) {

        FT_UInt glyph_index;
        glyph_index = FT_Get_Char_Index( face, op->str[i] );
        
        // Load glyph image into the slot (erase previous one)
        error = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT );
        if ( error )
            continue;
        
        // Convert to an anti-aliased bitmap
        error = FT_Render_Glyph( face->glyph, FT_RENDER_MODE_NORMAL );
        if ( error )
            continue;

        // Copy bitmap to target
        FT_GlyphSlot slot = face->glyph;
        FT_Bitmap* bitmap = &slot->bitmap;
        
        float horiBearingX = (slot->metrics.horiBearingX >> 6);
        float horiBearingY = (slot->metrics.horiBearingY >> 6);
        
        for (int ySource = 0; ySource < bitmap->rows; ySource++) {
            for (int xSource = 0; xSource < bitmap->width; xSource++) {
                unsigned char pixel = bitmap->buffer[ySource * bitmap->width + xSource];
                
                int xDest = pen.x + xSource + horiBearingX;
                int yDest = pen.y + ySource - horiBearingY;
                
                if (xDest < 0 || yDest < 0
                        || xDest >= op->bitmapSizeX
                        || yDest >= op->bitmapSizeY)
                    continue;
                
                op->bitmap[yDest*op->bitmapSizeX + xDest] = pixel;
            }
        }
        
        // Advance pen position
        pen.x += face->glyph->advance.x >> 6;
        pen.y += face->glyph->advance.y >> 6;
    }

    return true;
}

void font_cleanup_operation(FontBitmap* op)
{
    free(op->bitmap);
    op->bitmap = NULL;
}
