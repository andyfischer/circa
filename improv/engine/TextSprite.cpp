// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "Common.h"

#include "TextTexture.h"
#include "TextVbo.h"

#include "TextSprite.h"

void
TextSprite::init(RenderList* renderList, int font)
{
    textTexture = TextTexture::init(renderList);
    textVbo = TextVbo::init(renderList);
    
    textVbo->textTexture = textTexture;
    
    _width = 0;
    _font = font;
}

TextSprite::~TextSprite()
{
    textTexture->destroy();
    textVbo->destroy();
}

void
TextSprite::setFont(int f)
{
    _font = f;
}

void
TextSprite::setText(const char* text, int width)
{
    if (circa_string_equals(&_text, text) && width == _width)
        return;
    
    circa_set_string(&_text, text);
    _width = width;
    textTexture->rasterize(&_text, _font);
}

void
TextSprite::setPosition(int x, int y)
{
    textVbo->setPosition(x, y);
}
