// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "Common.h"

#include "TextTexture.h"
#include "TextVbo.h"

#include "TextSprite.h"

TextSprite*
TextSprite::create(RenderTarget* target)
{
    TextSprite* obj = new TextSprite();

    obj->textTexture = TextTexture::create(target);
    // obj->textVbo = TextVbo::create(target);
    
    // obj->textVbo->textTexture = obj->textTexture;

    obj->_font = 0;
    return obj;
}

void
TextSprite::destroy()
{
    textTexture->destroy();
    // textVbo->destroy();
    textTexture = NULL;
    textVbo = NULL;
}

bool
TextSprite::destroyed()
{
    return textTexture = NULL;
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
    // textTexture->rasterize(&_text, _font);
}

void
TextSprite::setPosition(int x, int y)
{
    // textVbo->setPosition(x, y);
}
void
TextSprite::setColor(Color color)
{
    // textVbo->color = color;
}
FontBitmap*
TextSprite::getMetrics()
{
    return &textTexture->metrics;
}

