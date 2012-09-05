// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

struct FontFace;

struct FontBitmap
{
    FontFace* face;    // Font face, input. 
    const char* str;  // String to render, input.

    int bitmapSizeX;
    int bitmapSizeY;
    
    // Result metrics
    float textWidth;
    float descent;
    float ascent;
    float originX; // Position of origin inside the bitmap
    float originY; // Position of origin inside the bitmap

    // Result bitmap.
    char* bitmap;
};

FontFace* font_load(const char* filename, int pixelHeight);
int font_get_face_height(FontFace* face);

// Update metric values using the input font and string.
void font_update_metrics(FontBitmap* op);

// Render using the input font and string, and the bitmap size.
bool font_render(FontBitmap* op);

void font_cleanup_operation(FontBitmap* op);
