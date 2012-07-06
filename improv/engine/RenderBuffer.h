// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

typedef struct UniformList {
    GLuint modelViewProjectionMatrix;
    GLuint normalMatrix;
    GLuint sampler;
} UniformList;

typedef struct Program {
    GLuint program;
    UniformList uniforms;
} Program;

enum
{
    ATTRIB_VERTEX,
    ATTRIB_NORMAL,
    ATTRIB_TEX_COORD,
    ATTRIB_COLOR,
    NUM_ATTRIBUTES
};


typedef int EntityId;

typedef struct RenderBuffer RenderBuffer;

void check_gl_error();

// Initialize the RenderBuffer. This will start making GL operations right away, so make
// sure the GL context is properly set up for this call and all rb_ calls.
RenderBuffer* rb_create();

// Update the size of the view, this is used when setting up the orthographic projection
// inside rb_render().
void rb_set_view_size(RenderBuffer* buffer, float w, float h);

void rb_remove_entity(RenderBuffer* buffer, EntityId id);

EntityId rb_add_text_container(RenderBuffer* buffer);

void rb_set_text_contents(RenderBuffer* buffer, EntityId id, int face, int width, const char* text);

EntityId rb_add_text_instance(RenderBuffer* buffer);
    
void rb_position_text(RenderBuffer* buffer, EntityId id, EntityId textureId, float x, float y);

void rb_render(RenderBuffer* buffer);

// ShaderUtils.m
bool load_shaders(Program* programStruct, const char* filename);
