// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

#include "plastic_common_headers.h"

#include "app.h"
#include "plastic_main.h"

GLuint load_shader(std::string const& vertFilename, std::string const& fragFilename)
{
    std::string vertText = circa::read_text_file(find_asset_file(vertFilename));
    std::string fragText = circa::read_text_file(find_asset_file(fragFilename));

    const char* vert = vertText.c_str();
    const char* frag = fragText.c_str();
    
    GLchar buf[256];
    GLuint vertShader, fragShader, program;
    GLint success;

    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, (const GLchar**) &vert, 0);
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertShader, sizeof(buf), 0, buf);
        app::error("Unable to compile vertex shader.");
    }

    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, (const GLchar**) &frag, 0);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragShader, sizeof(buf), 0, buf);
        app::error("Unable to compile fragment shader.");
    }

    program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, sizeof(buf), 0, buf);
        app::error("Unable to link shaders.\n");
    }

    return program;
}
