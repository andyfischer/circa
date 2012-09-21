// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "RenderEntity.h"

struct Texture : RenderEntity
{
    GLuint tex;
    
    int sizeX;
    int sizeY;

    bool hasTexture;

    static Texture* create(RenderTarget* target);

    virtual void destroy();
    virtual bool destroyed();

    void loadFromFile(const char* filename);
    void loadCheckerPattern(int w, int h);
    void size(float* x, float* y);
};
