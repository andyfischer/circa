// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "Line.h"

Line*
Line::init(RenderList* target)
{
    Line* obj = new Line();
    target->appendCommand(obj);
    glGenBuffers(1, &obj->bufid);
    obj->hasPosition = false;
    return obj;
}

void
Line::destroy()
{
    glDeleteBuffers(1, &bufid);
    bufid = 0;
}

bool
Line::destroyed()
{
    return bufid == 0;
}

void
Line::setPosition(float x1, float y1, float x2, float y2)
{
    GLfloat vertices[] = {
        x1, y1, 0,
        x2, y2, 0,
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, bufid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    check_gl_error();
    hasPosition = true;
}

void
Line::render(RenderList* target)
{
    if (!hasPosition)
        return;
    
    const int floatsPerVertex = 3;
    const int numVertices = 2;
    
    GLuint attribVertex = target->currentProgram()->attributes.vertex;
    
    glBindBuffer(GL_ARRAY_BUFFER, bufid);
    glEnableVertexAttribArray(attribVertex);
    glVertexAttribPointer(attribVertex, floatsPerVertex,
                          GL_FLOAT, GL_FALSE,
                          0, BUFFER_OFFSET(0));
    
    glUniform4f(target->currentProgram()->uniforms.color,
                color.r,color.g,color.b,color.a);
    
    glDrawArrays(GL_LINES, 0, numVertices);
    check_gl_error();
    
    // Cleaup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
