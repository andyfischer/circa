// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// TextTexture
//
// Object that holds a texture id for a rendered piece of text.

#pragma once

#include "RenderEntity.h"
#include "FontBitmap.h"

struct TextTexture : RenderEntity
{
    GLuint texid;

    FontBitmap metrics;
    int version;

    static TextTexture* create(RenderTarget* renderList);
    void rasterize(caValue* str, int font);

    virtual void destroy();
    virtual bool destroyed();
};
