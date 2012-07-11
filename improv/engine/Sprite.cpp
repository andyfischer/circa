// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "Common.h"
#include "Sprite.h"
#include "RenderTarget.h"

Sprite* Sprite::init(RenderTarget* target)
{
    Sprite* obj = new Sprite();

    // target->appendCommand(obj);

    glGenBuffers(1, &obj->vbo);
    obj->vboNeedsUpdate = true;
    obj->hasCustomSize = false;
    obj->blend = 0;
    return obj;
}

void Sprite::destroy()
{
    texture = NULL;
    glDeleteBuffers(1, &vbo);
    vbo = 0;
}

bool
Sprite::destroyed()
{
    return vbo == 0;
}

void Sprite::loadFromFile(const char* filename)
{
    // TODO
    //texture = [[Texture alloc] init:[RenderTarget instance]];
    //[texture loadFromFile:filename];
}

void Sprite::size(float* x, float* y)
{
    if (hasCustomSize) {
        *x = customSizeX;
        *y = customSizeY;
    } else if (texture != NULL) {
        *x = texture->sizeX;
        *y = texture->sizeY;
    }
}

void
Sprite::position(float* x, float* y)
{
    *x = posX;
    *y = posY;
}

void
Sprite::updateVbo()
{
    float sizeX,sizeY;
    size(&sizeX, &sizeY);

    GLfloat vertices[] = {
        // 3 floats for position, 2 for tex coord
        posX, posY,                 0.0, 0.0, 0.0,
        posX + sizeX, posY,         0.0, 1.0, 0.0,
        posX, posY + sizeY,         0.0, 0.0, 1.0,
        posX + sizeX, posY + sizeY, 0.0, 1.0, 1.0,
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    vboNeedsUpdate = false;
}

void Sprite::setPosition(float x, float y)
{
    if (posX == x && posY == y)
        return;
    
    posX = x;
    posY = y;
    vboNeedsUpdate = true;
}

void Sprite::setSize(float w, float h)
{
    if (customSizeX == w && customSizeY == h)
        return;
    
    customSizeX = w;
    customSizeY = h;
    hasCustomSize = true;
    vboNeedsUpdate = true;
}

void Sprite::render(RenderTarget* target)
{
#if 0
    if (texture == NULL || !texture->hasTexture)
        return;
    
    if (vboNeedsUpdate)
        updateVbo();
    
    const int floatsPerVertex = 5;
    
    GLuint attribVertex = target->currentProgram()->attributes.vertex;
    GLuint attribTexCoord = target->currentProgram()->attributes.tex_coord;
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(attribVertex);
    glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, floatsPerVertex*4, BUFFER_OFFSET(0));
    
    glEnableVertexAttribArray(attribTexCoord);
    glVertexAttribPointer(attribTexCoord, 2, GL_FLOAT, GL_FALSE, floatsPerVertex*4, BUFFER_OFFSET(12));
    
    // Bind texture 1
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->tex);
    glUniform1i(target->currentProgram()->uniforms.sampler, 0);
    
    // Bind texture 2
    if (texture2 != NULL && target->currentProgram()->uniforms.sampler2 != -1) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2->tex);
        glUniform1i(target->currentProgram()->uniforms.sampler2, 1);
    }
    
    // Color
    glUniform4f(target->currentProgram()->uniforms.color,
                color.r,color.g,color.b,color.a);
    glUniform1f(target->currentProgram()->uniforms.blend, blend);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // cleanup
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    check_gl_error();
#endif
}
