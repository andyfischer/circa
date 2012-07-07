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
TextSprite::setText(const char* text)
{
    if (circa_string_equals(&_text, text))
        return;
    
    circa_set_string(&_text, text);
    textTexture->rasterize(&_text, _font);
}

void
TextSprite::setPosition(int x, int y)
{
    textVbo->setPosition(x, y);
}
