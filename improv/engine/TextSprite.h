// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// TextSprite
//
// A piece of text that appears on the screen. Wraps a TextTexture and a TextVbo
// into one convenient object.

#include "circa/circa.h"

struct TextTexture;
struct TextVbo;

struct TextSprite
{
    TextTexture* textTexture;
    TextVbo* textVbo;
    circa::Value _text;
    int _font;

    void init(RenderList* rl, int font);
    ~TextSprite();

    void setFont(int font);
    void setText(const char* str);
    void setPosition(int x, int y);
};
