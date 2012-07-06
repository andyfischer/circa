// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// TextVbo
//
// Stores a vertex buffer for a single TextTexture.

#pragma once

#include "Common.h"
#include "RenderCommand.h"
#include "TextTexture.h"

struct TextVbo : RenderCommand
{
    TextTexture* textTexture;
    int textContainerVersionUsed;
    
    GLuint vbo;
    bool vboNeedsUpdate;
    
    float posX;
    float posY;
    
    Color color;

    void setPosition(float x, float y);
    void updateVbo();

    static TextVbo* init(RenderList* target);
    virtual void destroy();
    virtual bool destroyed();
    virtual void render(RenderList* renderList);
};
