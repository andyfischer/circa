// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "improv_common.h"
#include "circa/circa.h"

enum UniformType {
    VEC2 = 1,
    VEC3,
    VEC4,
    MAT2,
    MAT3,
    MAT4
};

struct Shader;

struct Context {
    Shader* currentShader;
};

Context g_context;

int next_power_of_two(int i)
{
    i--;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    return i + 1;
}

void gl_check_error()
{
#if DEBUG
    GLenum err = glGetError();
    if (err == GL_NO_ERROR)
        return;
    
    while (err != GL_NO_ERROR) {
        const char* str;
        switch (err) {
            case GL_INVALID_ENUM: str = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: str = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: str = "GL_INVALID_OPERATION"; break;
#ifndef NACL
            case GL_STACK_OVERFLOW: str = "GL_STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW: str = "GL_STACK_UNDERFLOW"; break;
#endif
            case GL_OUT_OF_MEMORY: str = "GL_OUT_OF_MEMORY"; break;
            default: str = "(enum not found)";
        }
        printf("OpenGL error: %s\n", str);
        err = glGetError();
    }
    assert(false);
#endif
}

struct Shader {
    GLuint programId;
    GLuint vertexId;
    GLuint fragmentId;
    circa::Value cachedAttributeLoc;
    circa::Value cachedUniformLoc;
    circa::Value defaultAttribList;

    Shader()
      : programId(0), vertexId(0), fragmentId(0)
    {
        circa_set_map(&cachedAttributeLoc);
        circa_set_map(&cachedUniformLoc);
        circa_set_list(&defaultAttribList, 0);
    }
    ~Shader()
    {
        glDeleteShader(vertexId);
        glDeleteShader(fragmentId);
        glDeleteProgram(programId);
    }

    void init()
    {
        programId = glCreateProgram();
        fragmentId = glCreateShader(GL_FRAGMENT_SHADER);
        vertexId = glCreateShader(GL_VERTEX_SHADER);
    }

    static void Release(void* ptr)
    {
        delete ((Shader*) ptr);
    }

    void copy_to(caValue* value)
    {
        circa_set_list(value, 1);
        circa_set_native_ptr(circa_index(value, 0), this, Shader::Release);
    }
    static Shader* get(caValue* value)
    {
        return (Shader*) circa_native_ptr(circa_index(value, 0));
    }
    int findAttribute(caValue* name)
    {
        caValue* cachedLoc = circa_map_get(&cachedAttributeLoc, name);

        if (cachedLoc != NULL)
            return circa_int(cachedLoc);

        int loc = glGetAttribLocation(programId, circa_symbol_text(name));
        circa_map_set_int(&cachedAttributeLoc, name, loc);

        return loc;
    }
    int findUniform(caValue* name)
    {
        caValue* cachedLoc = circa_map_get(&cachedUniformLoc, name);

        if (cachedLoc != NULL)
            return circa_int(cachedLoc);

        int loc = glGetUniformLocation(programId, circa_symbol_text(name));
        circa_map_set_int(&cachedUniformLoc, name, loc);

        return loc;
    }
};

struct VertexBuffer {
    GLuint id;
    int floatCount;
    GLenum drawType;
    circa::Value attribList;
    int floatsPerVertex;
    
    VertexBuffer()
      : id(0), floatCount(0), drawType(0), floatsPerVertex(0)
    {
    }
    ~VertexBuffer()
    {
        glDeleteBuffers(1, &id);
    }

    void init()
    {
        glGenBuffers(1, &id);
    }

    static void Release(void* ptr)
    {
        delete ((VertexBuffer*) ptr);
    }

    void copy_to(caValue* value)
    {
        circa_set_list(value, 1);
        circa_set_native_ptr(circa_index(value, 0), this, VertexBuffer::Release);
    }

    static VertexBuffer* get(caValue* value)
    {
        return (VertexBuffer*) circa_native_ptr(circa_index(value, 0));
    }
};

struct Texture
{
    GLuint id;
    int width;
    int height;

    Texture()
      : id(0), width(0), height(0)
    {
    }

    void init()
    {
        glGenTextures(1, &id);
    }

    ~Texture()
    {
        glDeleteTextures(1, &id);
    }

    static void Release(void* ptr)
    {
        delete ((Texture*) ptr);
    }

    void copy_to(caValue* value)
    {
        circa_set_list(value, 1);
        circa_set_native_ptr(circa_index(value, 0), this, Texture::Release);
    }

    void loadCheckerPattern(int w, int h)
    {
        w = next_power_of_two(w);
        h = next_power_of_two(h);
        
        char* data = (char*) malloc(w * h * 4);
        
        memset(data, 0xffffffff, w * h * 4);
        
        for (int x=0; x < w; x++)
            for (int y=0; y < h; y++) {
                unsigned* pixel = (unsigned*) &data[(x * h + y)*4];
                
                *pixel = ((x+y)%2) == 1 ? 0xffffffff : 0;
            }
         
        
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        free(data);
        
        width = w;
        height = h;
    }
};

bool check_shader_error(caVM* vm, GLuint shader)
{
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_FALSE)
        return false;

    GLint logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

    char* logData = (char*) malloc(logLength);

    glGetShaderInfoLog(shader, logLength, &logLength, logData);

    circa::Value msg;
    circa_set_string(&msg, "Shader error: ");
    circa_string_append(&msg, logData);
    circa_output_error_val(vm, &msg);

    free(logData);
    return true;
}

bool check_program_error(caVM* vm, GLuint program)
{
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_FALSE)
        return false;

    GLint logLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

    char* logData = (char*) malloc(logLength);

    glGetProgramInfoLog(program, logLength, &logLength, logData);

    circa::Value msg;
    circa_set_string(&msg, "Program error: ");
    circa_string_append(&msg, logData);
    circa_output_error_val(vm, &msg);

    free(logData);
    return true;
}

void shader_from_text(caVM* vm)
{
    //printf("Compiling shader..\n");
    caValue* vertexText = circa_input(vm, 0);
    caValue* fragmentText = circa_input(vm, 1);

    Shader* shader = new Shader();
    shader->init();
    
    const char* vertexStr = circa_string(vertexText);

    glShaderSource(shader->vertexId, 1, &vertexStr, NULL);
    glCompileShader(shader->vertexId);

    if (check_shader_error(vm, shader->vertexId)) {
        delete shader;
        return;
    }

    const char* fragmentStr = circa_string(fragmentText);

    glShaderSource(shader->fragmentId, 1, &fragmentStr, NULL);
    glCompileShader(shader->fragmentId);

    if (check_shader_error(vm, shader->fragmentId)) {
        delete shader;
        return;
    }

    gl_check_error();

    glAttachShader(shader->programId, shader->vertexId);
    glAttachShader(shader->programId, shader->fragmentId);

    glLinkProgram(shader->programId);

    if (check_program_error(vm, shader->programId)) {
        delete shader;
        return;
    }

    glValidateProgram(shader->programId);

    if (check_program_error(vm, shader->programId)) {
        delete shader;
        return;
    }

    gl_check_error();

    shader->copy_to(circa_output(vm));
}

void use_shader(caVM* vm)
{
    Shader* shader = Shader::get(circa_input(vm, 0));
    glUseProgram(shader->programId);

    g_context.currentShader = shader;
   
    gl_check_error();
}

void new_vertex_buffer(caVM* vm)
{
    VertexBuffer* buffer = new VertexBuffer();
    buffer->init();

    buffer->copy_to(circa_output(vm));
}

void VertexBuffer__set_data(caVM* vm)
{
    VertexBuffer* buffer = VertexBuffer::get(circa_input(vm, 0));
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);

    caValue* blob = circa_input(vm, 1);
    buffer->floatCount = circa_blob_size(blob) / 4;
    glBufferData(GL_ARRAY_BUFFER, circa_blob_size(blob), circa_blob(blob), GL_DYNAMIC_DRAW);
    gl_check_error();
}

void VertexBuffer__draw(caVM* vm)
{
    VertexBuffer* buffer = VertexBuffer::get(circa_input(vm, 0));

    if (buffer->drawType == 0)
        return circa_output_error(vm, "Buffer does not have draw_type assigned");

    if (circa_is_null(&buffer->attribList))
        return circa_output_error(vm, "Buffer does not have attribs assigned");

    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);

    int floatsSoFar = 0;

    for (int i=0; i < circa_length(&buffer->attribList); i++) {
        caValue* attrib = circa_index(&buffer->attribList, i);
        const char* name = circa_symbol_text(circa_index(attrib, 0));
        int count = circa_int(circa_index(attrib, 1));

        GLint vertexLoc = glGetAttribLocation(g_context.currentShader->programId, name);

        if (vertexLoc < 0) {
            circa::Value msg;
            circa_set_string(&msg, "Attrib not found in current program: ");
            circa_string_append(&msg, name);
            return circa_output_error_val(vm, &msg);
        }

        glEnableVertexAttribArray(vertexLoc);
        glVertexAttribPointer(vertexLoc, count, GL_FLOAT, GL_FALSE,
                              buffer->floatsPerVertex * 4,
                              BUFFER_OFFSET(4 * floatsSoFar));

        floatsSoFar += count;
        
        gl_check_error();
    }
    
    gl_check_error();
    
    glDrawArrays(buffer->drawType, 0, buffer->floatCount / buffer->floatsPerVertex);
    gl_check_error();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer__set_draw_type(caVM* vm)
{
    VertexBuffer* buffer = VertexBuffer::get(circa_input(vm, 0));

    if (circa_symbol_equals(circa_input(vm, 1), "triangles"))
        buffer->drawType = GL_TRIANGLES;
    else if (circa_symbol_equals(circa_input(vm, 1), "triangle_strip"))
        buffer->drawType = GL_TRIANGLE_STRIP;
    else
        return circa_output_error(vm, "Unrecognized draw type");
}

void VertexBuffer__set_attribs(caVM* vm)
{
    VertexBuffer* buffer = VertexBuffer::get(circa_input(vm, 0));
    circa_copy(circa_input(vm, 1), &buffer->attribList);

    buffer->floatsPerVertex = 0;

    if (circa_length(&buffer->attribList) == 0)
        return circa_output_error(vm, "Empty attrib list");

    for (int i=0; i < circa_length(&buffer->attribList); i++) {
        caValue* attrib = circa_index(&buffer->attribList, i);
        if (!circa_is_symbol(circa_index(attrib, 0)))
            return circa_output_error(vm, "Expected symbol in element 0");
        if (!circa_is_int(circa_index(attrib, 1)))
            return circa_output_error(vm, "Expected integer in element 1");
        buffer->floatsPerVertex += circa_int(circa_index(attrib, 1));
    }
}

void clear(caVM* vm)
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void uniform(caVM* vm, UniformType dataType)
{
    gl_check_error();
        
    if (g_context.currentShader == NULL)
        return circa_output_error(vm, "No active shader");

    caValue* name = circa_input(vm, 0);
    caValue* val = circa_input(vm, 1);

    int loc = g_context.currentShader->findUniform(name);
    
    if (loc < 0) {
        circa::Value msg;
        circa_set_string(&msg, "No uniform found with name: ");
        circa_string_append(&msg, circa_symbol_text(name));
        return circa_output_error_val(vm, &msg);
    }

    switch (dataType) {
    case VEC2: {
        const int count = 2;
        GLfloat data[count];

        for (int i=0; i < count; i++)
            data[i] = circa_float(circa_index(val, i));

        glUniform2fv(loc, 1, data);
        break;
    }
    case VEC3: {
        const int count = 3;
        GLfloat data[count];

        for (int i=0; i < count; i++)
            data[i] = circa_float(circa_index(val, i));

        glUniform3fv(loc, 1, data);
        break;
    }
    case VEC4: {
        const int count = 4;
        GLfloat data[count];

        for (int i=0; i < count; i++)
            data[i] = circa_float(circa_index(val, i));

        glUniform4fv(loc, 1, data);
        break;
    }
    case MAT2: {
        GLfloat data[4];

        for (int col=0; col < 2; col++)
            for (int row=0; row < 2; row++)
                data[col*2 + row] = circa_float(circa_index(circa_index(val, col), row));

        glUniformMatrix2fv(loc, 1, false, data);
        break;
    }
    case MAT3: {
        GLfloat data[9];

        for (int col=0; col < 3; col++)
            for (int row=0; row < 3; row++)
                data[col*3 + row] = circa_float(circa_index(circa_index(val, col), row));

        glUniformMatrix3fv(loc, 1, false, data);
        break;
    }
    case MAT4: {
        GLfloat data[16];

        for (int col=0; col < 4; col++)
            for (int row=0; row < 4; row++)
                data[col*4 + row] = circa_float(circa_index(circa_index(val, col), row));

        glUniformMatrix4fv(loc, 1, false, data);
        break;
    }
    }
    gl_check_error();
}

void uniform_vec2(caVM* vm) { uniform(vm, VEC2); }
void uniform_vec3(caVM* vm) { uniform(vm, VEC3); }
void uniform_vec4(caVM* vm) { uniform(vm, VEC4); }
void uniform_mat2(caVM* vm) { uniform(vm, MAT2); }
void uniform_mat3(caVM* vm) { uniform(vm, MAT3); }
void uniform_mat4(caVM* vm) { uniform(vm, MAT4); }

void new_texture(caVM* vm)
{
    Texture* texture = new Texture();
    texture->init();
    texture->loadCheckerPattern(100, 100);
    texture->copy_to(circa_output(vm));
}

void Texture__set_data(caVM* vm)
{
    Texture* texture = (Texture*) circa_native_ptr(circa_index(circa_input(vm, 0), 0));
    caValue* size = circa_input(vm, 1);
    caValue* blob = circa_input(vm, 2);

    float widthF;
    float heightF;
    circa_vec2(size, &widthF, &heightF);

    int width = int(widthF);
    int height = int(heightF);

    char* data = circa_blob(blob);
    uint32_t blobSize = circa_blob_size(blob);

    if (blobSize < (width*height))
        return circa_output_error(vm, "Blob does not have enough bytes for dimensions");
    
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void bind_texture(caVM* vm)
{
    if (g_context.currentShader == NULL)
        return circa_output_error(vm, "No active shader");
    
    Texture* texture = (Texture*) circa_native_ptr(circa_index(circa_input(vm, 0), 0));
    caValue* samplerName = circa_input(vm, 1);

    int samplerLoc = g_context.currentShader->findUniform(samplerName);
    
    if (samplerLoc < 0) {
        circa::Value msg;
        circa_set_string(&msg, "No uniform found with name: ");
        circa_string_append(&msg, circa_symbol_text(samplerName));
        return circa_output_error_val(vm, &msg);
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glUniform1i(samplerLoc, 0);
}

void unbind_texture(caVM* vm)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void gl_native_patch(caNativePatch* patch)
{
    circa_patch_function(patch, "shader_from_text", shader_from_text);
    circa_patch_function(patch, "new_vertex_buffer", new_vertex_buffer);
    circa_patch_function(patch, "VertexBuffer.set_data", VertexBuffer__set_data);
    circa_patch_function(patch, "VertexBuffer.draw", VertexBuffer__draw);
    circa_patch_function(patch, "VertexBuffer.set_draw_type", VertexBuffer__set_draw_type);
    circa_patch_function(patch, "VertexBuffer.set_attribs", VertexBuffer__set_attribs);
    circa_patch_function(patch, "use_shader", use_shader);
    circa_patch_function(patch, "clear", clear);
    circa_patch_function(patch, "uniform_mat2", uniform_mat2);
    circa_patch_function(patch, "uniform_mat3", uniform_mat3);
    circa_patch_function(patch, "uniform_mat4", uniform_mat4);
    circa_patch_function(patch, "uniform_vec2", uniform_vec2);
    circa_patch_function(patch, "uniform_vec3", uniform_vec3);
    circa_patch_function(patch, "uniform_vec4", uniform_vec4);
    circa_patch_function(patch, "new_texture", new_texture);
    circa_patch_function(patch, "Texture.set_data", Texture__set_data);
    circa_patch_function(patch, "bind_texture", bind_texture);
    circa_patch_function(patch, "unbind_texture", unbind_texture);
    circa_finish_native_patch(patch);
}
