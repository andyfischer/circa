// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include <OpenGL/gl.h>
#include <glm/glm.hpp>
#include "circa/circa.h"

#include <vector>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

struct RenderCommand;
struct RenderEntity;
struct RenderTarget;
struct ResourceManager;

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

struct RenderTarget
{
    // RenderCommands are pending commands to the device, they are run on every frame.
    // These objects are owned by RenderTarget.
    std::vector<RenderCommand*> commands;

    // RenderEntities store objects/data that is actively being used for drawing (and
    // may be referenced by RenderCommands). These objects are owned by RenderTarget.
    std::vector<RenderEntity*> entities;
    
    Program program;
    
    int viewportWidth, viewportHeight;
    glm::mat4 modelViewProjectionMatrix;
    glm::mat3 normalMatrix;

    circa::Value textRenderCache;
    circa::Value incomingCommands;

    RenderTarget();
    void setup(ResourceManager* resourceManager);
    void appendEntity(RenderEntity* entity);
    void appendCommand(RenderCommand* command);

    void setViewportSize(int w, int h);
    void render();

    Program* currentProgram();
};

void check_gl_error();
