// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// TextTexture
//
// Object that holds a texture id for a rendered piece of text.
//

#pragma once

#include "RenderData.h"
#include "FontBitmap.h"

struct TextTexture : RenderData
{
    GLuint texid;

    FontBitmap metrics;
    int version;

    static TextTexture* init(RenderList* renderList);
    void rasterize(caValue* str, int font);

    virtual void destroy();
    virtual bool destroyed();
};
