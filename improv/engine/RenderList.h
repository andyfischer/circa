// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include <OpenGL/gl.h>
#include <glm/glm.hpp>

#include <vector>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

struct RenderCommand;
struct RenderData;
struct RenderList;

struct AttributeList {
    GLuint vertex;
    GLuint tex_coord;
};

struct UniformList {
    GLuint modelViewProjectionMatrix;
    GLuint normalMatrix;
    GLuint sampler;
    GLuint sampler2;
    GLuint color;
    GLuint blend;
};

struct Program {
    GLuint program;
    AttributeList attributes;
    UniformList uniforms;
};

struct RenderList
{
    std::vector<RenderData*> renderData;
    std::vector<RenderCommand*> commands;
    
    // Current state
    Program* curProgram;
    
    glm::mat4 modelViewProjectionMatrix;
    glm::mat3 normalMatrix;

    void setup();
    void appendRenderData(RenderData* data);
    void appendCommand(RenderCommand* command);
    void render();
};

void check_gl_error();
