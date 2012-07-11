// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// TextVbo
//
// Stores a vertex buffer for a single TextTexture.

#pragma once

#include "Common.h"
#include "TextTexture.h"

struct Program;

struct TextVbo
{
    GLuint vbo;

    void update(TextTexture* texture, float posX, float posY);
    void render(Program* currentProgram, Color color);

    TextVbo();
    ~TextVbo();
};
