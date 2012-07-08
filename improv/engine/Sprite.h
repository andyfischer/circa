// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "RenderCommand.h"
#include "Texture.h"

struct Sprite : RenderCommand
{
    Texture* texture;
    Texture* texture2;
        
    Color color;
    float blend;

    float posX, posY;
    float customSizeX, customSizeY;
    bool hasCustomSize;
    GLuint vbo;
    bool vboNeedsUpdate;

    static Sprite* init(RenderTarget* target);

    void loadFromFile(const char* filename);
    void setPosition(float x, float y);
    void setSize(float w, float h);
    void size(float* x, float* y);
    void position(float* x, float* y);
    void updateVbo();

    virtual void render(RenderTarget* target);
    virtual void destroy();
    virtual bool destroyed();
};
