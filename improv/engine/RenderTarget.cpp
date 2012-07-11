// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "CircaBindings.h"
#include "RenderEntity.h"
#include "RenderTarget.h"
#include "TextTexture.h"
#include "TextVbo.h"
#include "ShaderUtils.h"

const bool CHECK_GL_ERROR = true;

RenderTarget::RenderTarget()
  : viewportWidth(0),
    viewportHeight(0)
{
    circa_set_map(&textRenderCache);
    circa_set_list(&incomingCommands, 0);

    name_rect = circa_to_name("rect");
    name_textSprite = circa_to_name("textSprite");
}

void
RenderTarget::setup(ResourceManager* resourceManager)
{
    load_shaders(resourceManager, "assets/shaders/Text", &program);
}

void
RenderTarget::sendCommand(caValue* command)
{
    circa_copy(command, circa_append(&incomingCommands));
}

caValue*
RenderTarget::getTextRender(caValue* args)
{
    caValue* value = circa_map_insert(&textRenderCache, args);

    if (circa_is_null(value)) {
        // Value doesn't exist in cache.
        TextTexture* texture = TextTexture::create(this);

        texture->setText(circa_index(args, 0));
        FontRef* font = (FontRef*) circa_object(circa_index(args, 1));
        texture->setFont(font->font_id);
        texture->update();

        circa_set_list(value, 2);
        circa_set_pointer(circa_index(value, 0), texture);
        texture->getSize(circa_index(value, 1));
    }

    return value;
}

void
RenderTarget::appendEntity(RenderEntity* command)
{
    entities.push_back(command);
}

void
RenderTarget::setViewportSize(int w, int h)
{
    viewportWidth = w;
    viewportHeight = h;
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

    // Run incoming commands
    for (size_t i=0; i < circa_count(&incomingCommands); i++) {
        caValue* command = circa_index(&incomingCommands, i);
        caName commandName = circa_name(circa_index(command, 0));

        if (commandName == name_textSprite) {
            TextTexture* texture = (TextTexture*) circa_get_pointer(circa_index(command, 1));

            caValue* position = circa_index(command, 2);
            caValue* color = circa_index(command, 3);

            TextVbo* textVbo = TextVbo::create(this);
            textVbo->textTexture = texture;
            textVbo->color = unpack_color(color);
            float x,y;
            circa_vec2(position, &x, &y);
            textVbo->setPosition(x,y);
            textVbo->render(this);

            delete textVbo;
        } else if (commandName == name_rect) {
            // TODO
        } else {
            printf("unrecognized command name: %s\n", circa_name_to_string(commandName));
        }
        
        check_gl_error();
    }

    circa_set_list(&incomingCommands, 0);
}

void
RenderTarget::flushDestroyedEntities()
{
    // Cleanup destroyed entities
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
