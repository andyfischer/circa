// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RenderCommand.h"
#include "RenderEntity.h"
#include "RenderTarget.h"
#include "ShaderUtils.h"

const bool CHECK_GL_ERROR = true;

void
RenderTarget::setup(ResourceManager* resourceManager)
{
    load_shaders(resourceManager, "assets/shaders/Text", &program);
}

void
RenderTarget::appendCommand(RenderCommand* command)
{
    commands.push_back(command);
}

void
RenderTarget::appendEntity(RenderEntity* command)
{
    entities.push_back(command);
}

void
RenderTarget::setViewportSize(int w, int h)
{
    modelViewProjectionMatrix = glm::ortho(0.0, double(w), double(h), 0.0, -1.0, 100.0);
}

void
RenderTarget::render()
{
    check_gl_error();

    // Setup render state
    glUseProgram(program.program);

    glUniformMatrix4fv(program.uniforms.modelViewProjectionMatrix,
            1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));

    // Cleanup destroyed entities
    {
        int numDestroyed = 0;
        for (size_t i=0; i < entities.size(); i++) {
            RenderEntity* entity = entities[i];
            if (entity->destroyed()) {
                delete entity;
                numDestroyed++;
                continue;
            }

            if (numDestroyed > 0)
                entities[i - numDestroyed] = entity;
        }
        entities.resize(entities.size() - numDestroyed);
    }

    // Run commands (cleaning up destroyed ones as we go)
    int numDestroyed = 0;
    for (size_t i=0; i < commands.size(); i++) {
        RenderCommand* command = commands[i];

        if (command->destroyed()) {
            delete command;
            numDestroyed++;
            continue;
        }

        if (numDestroyed > 0)
            commands[i - numDestroyed] = command;
    
        command->render(this);
        
        check_gl_error();
    }

    commands.resize(commands.size() - numDestroyed);
}

Program*
RenderTarget::currentProgram()
{
    return &program;
}

void check_gl_error()
{
    if (!CHECK_GL_ERROR)
        return;
    
    GLenum err = glGetError();
    if (err == GL_NO_ERROR)
        return;
    
    while (err != GL_NO_ERROR) {
        const char* str;
        switch (err) {
            case GL_INVALID_ENUM: str = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: str = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: str = "GL_INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW: str = "GL_STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW: str = "GL_STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY: str = "GL_OUT_OF_MEMORY"; break;
            default: str = "(enum not found)";
        }
        printf("OpenGL error: %s\n", str);
        err = glGetError();
    }
    assert(false);
    return;
}
