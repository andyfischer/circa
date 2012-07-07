// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "TextVbo.h"

TextVbo*
TextVbo::init(RenderList* target)
{
    TextVbo* obj = new TextVbo();
    target->appendCommand(obj);

    glGenBuffers(1, &obj->vbo);
    
    obj->vboNeedsUpdate = false;

    return obj;
}

void
TextVbo::destroy()
{
    glDeleteBuffers(1, &vbo);
    vbo = 0;
}

bool
TextVbo::destroyed()
{
    return vbo == 0;
}

void
TextVbo::setPosition(float x, float y)
{
    posX = x;
    posY = y;
}

void
TextVbo::updateVbo()
{
    assert(textTexture != NULL);
    check_gl_error();
    
    // Upload vertices for polygons
    //
    //       Top
    // Left  [0]  [1]
    //       [2]  [3]
    
    int sizeX = textTexture->metrics.bitmapSizeX;
    int sizeY = textTexture->metrics.bitmapSizeY;
    
    // Adjust position so that (X,Y) is at the origin
    float localPosX = posX - textTexture->metrics.originX;
    float localPosY = posY - textTexture->metrics.originY;
    
    GLfloat vertices[] = {
        // 3 floats for position, 2 for tex coord
        localPosX, localPosY, 0,                   0.0, 0.0,
        localPosX + sizeX, localPosY, 0,           1.0, 0.0,
        localPosX, localPosY + sizeY, 0,           0.0, 1.0,
        localPosX + sizeX, localPosY + sizeY, 0,   1.0, 1.0,
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    vboNeedsUpdate = false;
    textContainerVersionUsed = textTexture->version;
    
    check_gl_error();
}

void
TextVbo::render(RenderList* renderList)
{
    if (vboNeedsUpdate || textTexture->version != textContainerVersionUsed)
        updateVbo();
    
    check_gl_error();
    const int floatsPerVertex = 5;
    
    GLuint attribVertex = renderList->currentProgram()->attributes.vertex;
    GLuint attribTexCoord = renderList->currentProgram()->attributes.tex_coord;
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    check_gl_error();

    glEnableVertexAttribArray(attribVertex);
    glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, floatsPerVertex*4, BUFFER_OFFSET(0));
    check_gl_error();
    
    glEnableVertexAttribArray(attribTexCoord);
    glVertexAttribPointer(attribTexCoord, 2, GL_FLOAT, GL_FALSE, floatsPerVertex*4, BUFFER_OFFSET(12));
    check_gl_error();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textTexture->texid);
    check_gl_error();
    
    glUniform1i(renderList->currentProgram()->uniforms.sampler, 0);
    glUniform4f(renderList->currentProgram()->uniforms.color,
                color.r, color.g, color.b, color.a);
    check_gl_error();
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    check_gl_error();
}

