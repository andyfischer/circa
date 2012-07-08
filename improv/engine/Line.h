// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "Common.h"
#include "RenderCommand.h"

struct Line : RenderCommand
{
    GLuint bufid;
    bool hasPosition;
    
    Color color;

    static Line* init(RenderTarget* target);
    void setPosition(float x1, float y1, float x2, float y2);

    virtual void render(RenderTarget* target);
    virtual void destroy();
    virtual bool destroyed();
};
