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

    FontFace* font;
    circa::Value text;
    bool needsRasterize;
    int version;
    FontBitmap metrics;

    static TextTexture* create(RenderTarget* renderList);

    void setFont(FontFace* font);
    void setText(caValue* text);

    float width();
    float height();

    void update();

    virtual void destroy();
    virtual bool destroyed();
};
