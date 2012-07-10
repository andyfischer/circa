// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include <OpenGL/gl.h>
#include <glm/glm.hpp>
#include "circa/circa.h"

#include <vector>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

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
    void sendCommand(caValue* command);

    caValue* getTextRender(caValue* args);

    void setViewportSize(int w, int h);
    void render();
    void flushDestroyedEntities();

    Program* currentProgram();

    caName name_textSprite;
    caName name_rect;
};

void check_gl_error();
