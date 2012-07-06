// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "RenderData.h"

struct Texture : RenderData
{
    GLuint tex;
    
    int sizeX;
    int sizeY;

    bool hasTexture;

    void init(RenderList* target);
    void destroy();
    bool destroyed();

    void loadFromFile(const char* filename);
    void loadCheckerPattern(int w, int h);
    void size(float* x, float* y);
};
