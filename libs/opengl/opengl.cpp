// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "../shared/opengl.h"

using namespace circa;

extern "C" {

const char* gl_to_string(GLenum glenum)
{
    switch (glenum) {
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        case GL_RGBA: return "GL_RGBA";
        case GL_BGRA: return "GL_BGRA";
    }
    return "";
}

bool gl_check_error(circa::EvalContext* cxt, circa::Term* term)
{
    GLenum errcode = glGetError();
    if (errcode == GL_NO_ERROR)
        return false;

    const char* err = gl_to_string(errcode);

    circa::error_occurred(cxt, term, std::string("OpenGL reported error: ") + err);
    return true;
}

CA_FUNCTION(new_texture_handle)
{
    // TODO: Need to do garbage collection to reclaim IDs
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    set_int(OUTPUT, texture_id);
}

CA_FUNCTION(opengl__draw_texture_as_quad)
{
    int texture_id = as_int(INPUT(0));
    float x1(0), y1(0), x2(0), y2(0);
    get_rect(INPUT(1), &x1, &y1, &x2, &y2);

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex3f(x1, y1, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(x2, y1, 0);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(x2, y2, 0);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(x1, y2, 0);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
}

CA_FUNCTION(opengl__load_shader)
{
    std::string vertText = circa::storage::read_text_file_as_str(STRING_INPUT(0));
    std::string fragText = circa::storage::read_text_file_as_str(STRING_INPUT(1));

    const char* vert = vertText.c_str();
    const char* frag = fragText.c_str();
    
    GLuint vertShader, fragShader, program;
    GLint success;

    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, (const GLchar**) &vert, 0);
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar buf[2024];
        glGetShaderInfoLog(vertShader, sizeof(buf), 0, buf);
        ERROR_OCCURRED(std::string("Unable to compile vertex shader: ")+buf);
    }

    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, (const GLchar**) &frag, 0);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar buf[2024];
        glGetShaderInfoLog(fragShader, sizeof(buf), 0, buf);
        ERROR_OCCURRED(std::string("Unable to compile fragment shader: ")+buf);
    }

    program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar buf[256];
        glGetProgramInfoLog(program, sizeof(buf), 0, buf);
        ERROR_OCCURRED("Unable to link shaders.");
    }

    set_int(OUTPUT, program);
}

CA_FUNCTION(opengl__shader_quad)
{
    glUseProgram(INT_INPUT(0));

    List& uniforms = *as_list(INPUT(2));
    for (int i=0; i < uniforms.length(); i++)
        glUniform1f(as_int(list_get_index(uniforms[i], 0)), to_float(list_get_index(uniforms[i], 1)));

    float x1(0), y1(0), x2(0), y2(0);
    get_rect(INPUT(1), &x1, &y1, &x2, &y2);

    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex3f(x1, y1, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(x2, y1, 0);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(x2, y2, 0);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(x1, y2, 0);
    glEnd();

    glUseProgram(0);
    gl_check_error(CONTEXT, CALLER);
}

CA_FUNCTION(opengl__get_uniform_location)
{
    set_int(OUTPUT, glGetUniformLocation(INT_INPUT(0), STRING_INPUT(1)));
    gl_check_error(CONTEXT, CALLER);
}
CA_FUNCTION(opengl__uniform)
{
    glUniform1f(INT_INPUT(0), FLOAT_INPUT(1));
    gl_check_error(CONTEXT, CALLER);
}

void on_load(Branch* kernel)
{
}

} // extern "C"
