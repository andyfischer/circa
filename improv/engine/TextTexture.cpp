// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "Common.h"
#include "TextTexture.h"
#include "RenderTarget.h"

TextTexture*
TextTexture::create(RenderTarget* renderList)
{
    TextTexture* obj = new TextTexture();
    renderList->appendEntity(obj);

    glGenTextures(1, &obj->texid);

    obj->font = 0;
    circa_set_null(&obj->text);
    obj->needsRasterize = false;
    obj->version = 1;
    return obj;
}
void
TextTexture::setFont(FontFace* font)
{
    if (this->font == font)
        return;
    this->font = font;
    needsRasterize = true;
}

void
TextTexture::setText(caValue* text)
{
    if (circa_equals(&this->text, text))
        return;

    circa_copy(text, &this->text);
    needsRasterize = true;
}

float
TextTexture::width()
{
    return metrics.textWidth;
}
float
TextTexture::height()
{
    return metrics.ascent + metrics.descent;
}

void
TextTexture::destroy()
{
    glDeleteTextures(1, &texid);
    texid = 0;
}

bool
TextTexture::destroyed()
{
    return texid == 0;
}

void
TextTexture::update()
{
    if (font == 0) {
        Log("TextTexture::rasterize called with null font");
        return;
    }

    metrics.str = circa_string(&text);
    metrics.face = font;
    
    font_update_metrics(&metrics);
    
    // Increase size to next power of two
    metrics.bitmapSizeX = NextPowerOfTwo(metrics.bitmapSizeX + 2);
    metrics.bitmapSizeY = NextPowerOfTwo(metrics.bitmapSizeY + 2);

    font_render(&metrics);
    
    // Load the result into the GL texture
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D,
                 0,            // level
                 GL_ALPHA,     // internal format
                 metrics.bitmapSizeX, metrics.bitmapSizeY, // width/height
                 0,            // border
                 GL_ALPHA,     // format
                 GL_UNSIGNED_BYTE, // type
                 metrics.bitmap);
    
    font_cleanup_operation(&metrics);
    
    version++;
    
    check_gl_error();
}
