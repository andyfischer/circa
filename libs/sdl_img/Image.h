// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "../shared/opengl.h"

struct Image
{
    int width;
    int height;
    char* pixels;
    GLenum pixelFormat;
    GLenum internalFormat;

    Image() : pixels(NULL)
    {
    }
    ~Image()
    {
        free(pixels);
    }
};
