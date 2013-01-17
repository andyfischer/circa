// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include <OpenGL/gl.h>
#include <glm/glm.hpp>
#include "circa/circa.h"

#include <vector>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

struct RenderEntity;
struct RenderList;
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

struct RenderList
{
    // RenderEntities store objects/data that is actively being used for drawing (and
    // may be referenced by RenderCommands). These objects are owned by RenderList.
    std::vector<RenderEntity*> entities;
    
    Program textProgram;
    Program geomProgram;
    Program* currentProgram;
    
    int viewportWidth, viewportHeight;
    glm::mat4 modelViewProjectionMatrix;
    glm::mat3 normalMatrix;

    circa::Value textRenderCache;
    circa::Value incomingCommands;

    // Current gpu state
    GLuint textVbo;
    GLuint geomVbo;

    RenderList();
    void setup(ResourceManager* resourceManager);
    void appendEntity(RenderEntity* entity);
    void sendCommand(caValue* command);

    void setViewportSize(int w, int h);
    void switchProgram(Program* program);
    void startFrame();
    void submitFrame();
    void flushDestroyedEntities();

    caSymbol name_textSprite;
    caSymbol name_rect;
    caSymbol name_lines;
    caSymbol name_polygon;
    caSymbol name_AlignHCenter;
    caSymbol name_AlignVCenter;
};

void check_gl_error();

void RenderList_moduleLoad(caNativePatch* module);
