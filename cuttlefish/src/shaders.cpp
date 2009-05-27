// Copyright 2009 Paul Hodge

#include "common_headers.h"

#include "SDL_opengl.h"

GLuint load_shader(GLenum shaderType, std::string const& glslSource)
{
    while (glGetError() != GL_NO_ERROR);

    GLenum err = GL_NO_ERROR;

    GLuint shader = glCreateShader(shaderType);
    err = glGetError();
    if (err != GL_NO_ERROR)
        std::cout << "error after glCreateShader" << std::endl;

    const char* glslSource_cstr = glslSource.c_str();
    glShaderSource(shader, 1, (const GLchar**) &glslSource_cstr, NULL);

    err = glGetError();
    if (err != GL_NO_ERROR)
        std::cout << "error after glShaderSource" << std::endl;
    
    glCompileShader(shader);

    err = glGetError();
    if (err != GL_NO_ERROR)
        std::cout << "error after glCompileShader" << std::endl;

    // TODO: use glGetShaderiv to get GL_COMPILE_STATUS

    return shader;
}
