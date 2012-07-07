// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "Common.h"
#include "RenderList.h"
#include "ResourceManager.h"

void compile_shader(GLenum type, caValue* contents, caValue* shaderOut);
bool link_program(GLuint prog);
bool validate_program(GLuint prog);

bool load_shaders(ResourceManager* resources, const char* baseFilename, Program* programStruct)
{
    Log("Loading shader: %s", baseFilename);
    
    check_gl_error();

    // Create shader program.
    GLuint program = glCreateProgram();
    programStruct->program = program;
    
    // Load vertex shader.
    circa::Value vertShaderPathname;
    circa_set_string(&vertShaderPathname, baseFilename);
    circa_string_append(&vertShaderPathname, ".vsh");

    circa::Value vertShader;
    resources->loadAsText(&vertShaderPathname, &vertShader);

    if (circa_is_error(&vertShader)) {
        Log("Failed to load vertex shader: %v", &vertShaderPathname);
        return false;
    }

    // Compile vertex shader
    circa::Value vertShaderIndex;
    compile_shader(GL_VERTEX_SHADER, &vertShader, &vertShaderIndex);

    if (circa_is_error(&vertShaderIndex)) {
        Log("Failed to compile vertex shader: %v", &vertShaderPathname);
        return false;
    }

    // Load fragment shader
    circa::Value fragShaderPathname;
    circa_set_string(&fragShaderPathname, baseFilename);
    circa_string_append(&fragShaderPathname, ".fsh");

    circa::Value fragShader;
    resources->loadAsText(&fragShaderPathname, &fragShader);

    if (circa_is_error(&fragShader)) {
        Log("Failed to load fragment shader: %v", &fragShaderPathname);
        return false;
    }

    // Compile fragment shader
    circa::Value fragShaderIndex;
    compile_shader(GL_FRAGMENT_SHADER, &fragShader, &fragShaderIndex);

    if (circa_is_error(&fragShaderIndex)) {
        Log("Failed to compile frag shader: %v", &fragShaderPathname);
        return false;
    }
    
    check_gl_error();

    int vertShaderHandle = circa_int(&vertShader);
    int fragShaderHandle = circa_int(&fragShader);
    
    // Attach vertex shader to program.
    glAttachShader(program, vertShaderHandle);
    
    // Attach fragment shader to program.
    glAttachShader(program, fragShaderHandle);
    check_gl_error();
    
    // Link program.
    if (!link_program(program)) {
        Log("Failed to link shaders: %s", baseFilename);
        
        glDeleteShader(vertShaderHandle);
        glDeleteShader(fragShaderHandle);
        if (program) {
            glDeleteProgram(program);
            program = 0;
        }
        
        check_gl_error();
        return false;
    }
    check_gl_error();
    
    // Get attribute locations
    programStruct->attributes.vertex =
        glGetAttribLocation(program, "vertex");
    programStruct->attributes.tex_coord =
        glGetAttribLocation(program, "tex_coord");
    check_gl_error();
    
    // Get uniform locations.
    programStruct->uniforms.modelViewProjectionMatrix =
        glGetUniformLocation(program, "modelViewProjectionMatrix");
    programStruct->uniforms.normalMatrix = glGetUniformLocation(program, "normalMatrix");
    programStruct->uniforms.sampler = glGetUniformLocation(program, "sampler");
    programStruct->uniforms.sampler2 = glGetUniformLocation(program, "sampler2");
    programStruct->uniforms.color = glGetUniformLocation(program, "color");
    programStruct->uniforms.blend = glGetUniformLocation(program, "blend");

    check_gl_error();
    
    return true;
}

void compile_shader(GLenum type, caValue* contents, caValue* shaderOut)
{
    GLint status;
    GLuint shader;
    
    shader = glCreateShader(type);
    const char* source = circa_string(contents);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    check_gl_error();
    
#if 0
#if defined(DEBUG)
    GLint logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(shader, logLength, &logLength, log);
        Log("Shader compile log for %@:\n%s", file, log);
        free(log);
    }
#endif
#endif
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        glDeleteShader(shader);
        circa_set_error(shaderOut, "Failed to compile");
    }

    check_gl_error();
    circa_set_int(shaderOut, shader);
}

bool link_program(GLuint prog)
{
    check_gl_error();

    GLint status;
    glLinkProgram(prog);
    check_gl_error();
    
#if defined(DEBUG)
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        Log("Program link log:\n%s", log);
        free(log);
        return false;
    }
#endif
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    check_gl_error();

    if (status == 0)
        return false;
    
    return true;
}

bool validate_program(GLuint prog)
{
    check_gl_error();

    GLint logLength, status;
    
    glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        Log("Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    check_gl_error();

    if (status == 0)
        return false;
    
    return true;
}
