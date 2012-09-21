// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cstring>

#include "Common.h"
#include "Texture.h"

Texture*
Texture::create(RenderTarget* target)
{
    Texture* obj = new Texture();
    target->appendEntity(obj);

    glGenTextures(1, &obj->tex);
    obj->hasTexture = false;
    obj->sizeX = 0;
    obj->sizeY = 0;
    return obj;
}

void Texture::destroy()
{
    glDeleteTextures(1, &tex);
    tex = 0;
}

bool
Texture::destroyed()
{
    return tex == 0;
}

void
Texture::loadFromFile(const char * filename)
{
    // TODO
}

void
Texture::loadCheckerPattern(int w, int h)
{
    w = NextPowerOfTwo(w);
    h = NextPowerOfTwo(h);
    
    char* data = (char*) malloc(w * h * 4);
    
    memset(data, 0xffffffff, w * h * 4);
    
    for (int x=0; x < w; x++)
        for (int y=0; y < h; y++) {
            unsigned* pixel = (unsigned*) &data[(x * h + y)*4];
            
            *pixel = ((x+y)%2) == 1 ? 0xffffffff : 0;
        }
     
    
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    free(data);
    
    sizeX = w;
    sizeY = h;
    hasTexture = true;
}

void
Texture::size(float* x, float* y)
{
    *x = sizeX;
    *y = sizeY;
}
