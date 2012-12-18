// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RenderEntity.h"
#include "RenderList.h"
#include "TextTexture.h"
#include "ShaderUtils.h"
#include "triangulate.h"

#ifdef DEBUG
const bool CHECK_GL_ERROR = true;
#else
const bool CHECK_GL_ERROR = false;
#endif

void TextVbo_Update(GLuint vbo, TextTexture* texture, float posX, float posY);
void TextVbo_Render(GLuint vbo, Program* program, TextTexture* texture, caValue* color);
void Rect_Update(GLuint vbo, float x1, float y1, float x2, float y2);
void Rect_Render(GLuint vbo, Program* program, caValue* color);
void Lines_Update(GLuint vbo, caValue* points);
void Lines_Render(GLuint vbo, Program* program, int vertexCount, caValue* color);
int Ngon_Update(GLuint vbo, caValue* points);
void Triangles_Render(GLuint vbo, Program* program, int vertexCount, caValue* color);

RenderList::RenderList()
  : viewportWidth(0),
    viewportHeight(0)
{
    circa_set_map(&textRenderCache);
    circa_set_list(&incomingCommands, 0);

    name_rect = circa_to_name("rect");
    name_textSprite = circa_to_name("textSprite");
    name_lines = circa_to_name("lines");
    name_polygon = circa_to_name("polygon");
    name_AlignHCenter = circa_to_name("AlignHCenter");
    name_AlignVCenter = circa_to_name("AlignVCenter");
}

void
RenderList::setup(ResourceManager* resourceManager)
{
    load_shaders(resourceManager, "assets/shaders/Text", &textProgram);
    load_shaders(resourceManager, "assets/shaders/Geom", &geomProgram);

    glGenBuffers(1, &textVbo);
    glGenBuffers(1, &geomVbo);
}

void
RenderList::sendCommand(caValue* command)
{
    circa_copy(command, circa_append(&incomingCommands));
}

caValue*
RenderList::getTextRender(caValue* key)
{
    caValue* value = circa_map_insert(&textRenderCache, key);

    if (circa_is_null(value)) {
        printf("Re-rendering text with key: ");
        key->dump();
        
        // Value doesn't exist in cache.
        TextTexture* texture = TextTexture::create(this);

        texture->setText(circa_index(key, 0));
        texture->setFont((FontFace*) circa_get_pointer(circa_index(key, 1)));
        texture->update();

        circa_set_list(value, 2);
        circa_set_pointer(circa_index(value, 0), texture);
        circa_set_vec2(circa_index(value, 1), texture->width(), texture->height());
    }

    return value;
}

void
RenderList::appendEntity(RenderEntity* command)
{
    entities.push_back(command);
}

void
RenderList::setViewportSize(int w, int h)
{
    viewportWidth = w;
    viewportHeight = h;
    modelViewProjectionMatrix = glm::ortho(0.0, double(w), double(h), 0.0, -1.0, 100.0);
}

void
RenderList::switchProgram(Program* program)
{
    if (currentProgram == program)
        return;

    currentProgram = program;
    glUseProgram(program->program);

    glUniformMatrix4fv(program->uniforms.modelViewProjectionMatrix,
            1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
}

void
RenderList::render()
{
    check_gl_error();

    // Run incoming commands
    for (size_t commandIndex=0; commandIndex < circa_count(&incomingCommands); commandIndex++) {
        caValue* command = circa_index(&incomingCommands, (int) commandIndex);
        caName commandName = circa_name(circa_index(command, 0));

        if (commandName == name_textSprite) {
            // Draw a text label
            switchProgram(&textProgram);

            TextTexture* texture = (TextTexture*) circa_get_pointer(circa_index(command, 1));

            caValue* position = circa_index(command, 2);
            caValue* color = circa_index(command, 3);
            caValue* args = circa_index(command, 4);

            float posX, posY;
            circa_vec2(position, &posX, &posY);

            // Handle extra arguments
            for (int i=0; i < circa_count(args); i++) {
                caName name = circa_first_name(circa_index(args, i));
                if (name == name_AlignHCenter) {
                    posX -= texture->width() / 2;
                } else if (name == name_AlignVCenter) {
                    posY += texture->height() / 2;
                }
            }

            TextVbo_Update(textVbo, texture, posX, posY);
            TextVbo_Render(textVbo, currentProgram, texture, color);

        } else if (commandName == name_rect) {
            // Draw a solid rectangle
            switchProgram(&geomProgram);
            caValue* pos = circa_index(command, 1);
            caValue* color = circa_index(command, 2);
            float x1, y1, x2, y2;
            circa_vec4(pos, &x1, &y1, &x2, &y2);

            Rect_Update(geomVbo, x1, y1, x2, y2);
            Rect_Render(geomVbo, currentProgram, color);
        } else if (commandName == name_lines) {
            // Draw a list of lines.
            switchProgram(&geomProgram);
            caValue* points = circa_index(command, 1);
            caValue* color = circa_index(command, 2);
            int count = circa_count(points);

            Lines_Update(geomVbo, points);
            Lines_Render(geomVbo, currentProgram, count, color);
        } else if (commandName == name_polygon) {
            // Triangulate and draw an N-gon.
            switchProgram(&geomProgram);
            caValue* points = circa_index(command, 1);
            caValue* color = circa_index(command, 2);
            
            int count = Ngon_Update(geomVbo, points);
            Triangles_Render(geomVbo, currentProgram, count, color);

        } else {
            printf("unrecognized command name: %s\n", circa_name_to_string(commandName));
        }
        
        check_gl_error();
    }

    circa_set_list(&incomingCommands, 0);
}

void
RenderList::flushDestroyedEntities()
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

void TextVbo_Update(GLuint vbo, TextTexture* textTexture, float posX, float posY)
{
    assert(textTexture != NULL);
    check_gl_error();
    
    // Upload vertices for polygons
    //
    //       Top
    // Left  [0]  [1]
    //       [2]  [3]
    
    int sizeX = textTexture->metrics.bitmapSizeX;
    int sizeY = textTexture->metrics.bitmapSizeY;
    
    // Adjust position so that (X,Y) is at the origin
    float localPosX = posX - textTexture->metrics.originX;
    float localPosY = posY - textTexture->metrics.originY;
    
    GLfloat vertices[] = {
        // 3 floats for position, 2 for tex coord
        localPosX, localPosY, 0,                   0.0, 0.0,
        localPosX + sizeX, localPosY, 0,           1.0, 0.0,
        localPosX, localPosY + sizeY, 0,           0.0, 1.0,
        localPosX + sizeX, localPosY + sizeY, 0,   1.0, 1.0,
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    check_gl_error();
}

void TextVbo_Render(GLuint vbo, Program* program, TextTexture* textTexture, caValue* color)
{
    check_gl_error();
    const int floatsPerVertex = 5;
    const int vertexCount = 4;
    
    GLuint attribVertex = program->attributes.vertex;
    GLuint attribTexCoord = program->attributes.tex_coord;
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    check_gl_error();

    glEnableVertexAttribArray(attribVertex);
    glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE,
            floatsPerVertex*vertexCount, BUFFER_OFFSET(0));
    check_gl_error();
    
    glEnableVertexAttribArray(attribTexCoord);
    glVertexAttribPointer(attribTexCoord, 2, GL_FLOAT, GL_FALSE,
            floatsPerVertex*vertexCount, BUFFER_OFFSET(12));
    check_gl_error();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textTexture->texid);
    check_gl_error();
    
    glUniform1i(program->uniforms.sampler, 0);

    float r, g, b, a;
    circa_vec4(color, &r, &g, &b, &a);
    glUniform4f(program->uniforms.color, r, g, b, a);
    check_gl_error();
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);

    // cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    check_gl_error();
}

void Rect_Update(GLuint vbo, float x1, float y1, float x2, float y2)
{
    GLfloat vertices[] = {
        // 3 floats for position, 2 for tex coord
        x1, y1, 0,          0.0, 0.0,
        x2, y1, 0,          1.0, 0.0,
        x1, y2, 0,          0.0, 1.0,
        x2, y2, 0,          1.0, 1.0,
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Rect_Render(GLuint vbo, Program* program, caValue* color)
{
    const int floatsPerVertex = 5;
    const int vertexCount = 4;

    GLuint attribVertex = program->attributes.vertex;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(attribVertex);
    glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE,
            floatsPerVertex*vertexCount, BUFFER_OFFSET(0));
    check_gl_error();
    
    float r, g, b, a;
    circa_vec4(color, &r, &g, &b, &a);
    glUniform4f(program->uniforms.color, r, g, b, a);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);

    // cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Lines_Update(GLuint vbo, caValue* points)
{
    int count = circa_count(points);
    const int floatsPerVertex = 3;

    int verticesSize = sizeof(GLfloat) * floatsPerVertex * count;
    GLfloat* vertices = (GLfloat*) malloc(verticesSize);

    for (int i=0; i < count; i++) {
        float x,y;
        circa_vec2(circa_index(points, i), &x, &y);
        vertices[i*3 + 0] = x;
        vertices[i*3 + 1] = y;
        vertices[i*3 + 2] = 0.0;
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    free(vertices);
}
void Lines_Render(GLuint vbo, Program* program, int vertexCount, caValue* color)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLuint attribVertex = program->attributes.vertex;
    glEnableVertexAttribArray(attribVertex);
    glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    check_gl_error();
    
    float r, g, b, a;
    circa_vec4(color, &r, &g, &b, &a);
    glUniform4f(program->uniforms.color, r, g, b, a);

    glDrawArrays(GL_LINES, 0, vertexCount);

    // cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int Ngon_Update(GLuint vbo, caValue* points)
{
    // Triangulate
    circa::Value triangulatedPoints;
    circa_set_list(&triangulatedPoints, 0);
    Triangulate::Process(points, &triangulatedPoints);

    // Upload vertices
    int count = circa_count(&triangulatedPoints);
    const int floatsPerVertex = 3;

    int verticesSize = sizeof(GLfloat) * floatsPerVertex * count;
    GLfloat* vertices = (GLfloat*) malloc(verticesSize);

    for (int i=0; i < count; i++) {
        float x,y;
        circa_vec2(circa_index(&triangulatedPoints, i), &x, &y);
        vertices[i*3 + 0] = x;
        vertices[i*3 + 1] = y;
        vertices[i*3 + 2] = 0.0;
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    free(vertices);

    return count;
}
void Triangles_Render(GLuint vbo, Program* program, int vertexCount, caValue* color)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLuint attribVertex = program->attributes.vertex;
    glEnableVertexAttribArray(attribVertex);
    glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    check_gl_error();
    
    float r, g, b, a;
    circa_vec4(color, &r, &g, &b, &a);
    glUniform4f(program->uniforms.color, r, g, b, a);

    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#if 0
void
Sprite::updateVbo()
{
    float sizeX,sizeY;
    size(&sizeX, &sizeY);

    GLfloat vertices[] = {
        // 3 floats for position, 2 for tex coord
        posX, posY,                 0.0, 0.0, 0.0,
        posX + sizeX, posY,         0.0, 1.0, 0.0,
        posX, posY + sizeY,         0.0, 0.0, 1.0,
        posX + sizeX, posY + sizeY, 0.0, 1.0, 1.0,
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    vboNeedsUpdate = false;
}

void Sprite::setPosition(float x, float y)
{
    if (posX == x && posY == y)
        return;
    
    posX = x;
    posY = y;
    vboNeedsUpdate = true;
}

void Sprite::setSize(float w, float h)
{
    if (customSizeX == w && customSizeY == h)
        return;
    
    customSizeX = w;
    customSizeY = h;
    hasCustomSize = true;
    vboNeedsUpdate = true;
}

void Sprite::render(RenderList* target)
{
    if (texture == NULL || !texture->hasTexture)
        return;
    
    if (vboNeedsUpdate)
        updateVbo();
    
    const int floatsPerVertex = 5;
    
    GLuint attribVertex = target->currentProgram()->attributes.vertex;
    GLuint attribTexCoord = target->currentProgram()->attributes.tex_coord;
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(attribVertex);
    glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, floatsPerVertex*4, BUFFER_OFFSET(0));
    
    glEnableVertexAttribArray(attribTexCoord);
    glVertexAttribPointer(attribTexCoord, 2, GL_FLOAT, GL_FALSE, floatsPerVertex*4, BUFFER_OFFSET(12));
    
    // Bind texture 1
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->tex);
    glUniform1i(target->currentProgram()->uniforms.sampler, 0);
    
    // Bind texture 2
    if (texture2 != NULL && target->currentProgram()->uniforms.sampler2 != -1) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2->tex);
        glUniform1i(target->currentProgram()->uniforms.sampler2, 1);
    }
    
    // Color
    glUniform4f(target->currentProgram()->uniforms.color,
                color.r,color.g,color.b,color.a);
    glUniform1f(target->currentProgram()->uniforms.blend, blend);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // cleanup
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    check_gl_error();
}
#endif

// Circa bindings
void RenderList__getTextRender(caStack* stack)
{
    RenderList* target = (RenderList*) circa_handle_get_object(circa_input(stack, 0));
    caValue* args = circa_input(stack, 1);
    caValue* cachedRender = target->getTextRender(args);
    circa_copy(cachedRender, circa_output(stack, 0));
}
void RenderList__sendCommand(caStack* stack)
{
    RenderList* target = (RenderList*) circa_handle_get_object(circa_input(stack, 0));
    caValue* command = circa_input(stack, 1);
    target->sendCommand(command);
}
void RenderList__getViewportSize(caStack* stack)
{
    RenderList* target = (RenderList*) circa_handle_get_object(circa_input(stack, 0));
    circa_set_vec2(circa_output(stack, 0), target->viewportWidth, target->viewportHeight);
}

void RenderList_moduleLoad(caNativeModule* module)
{
    circa_patch_function(module, "RenderList.getTextRender", RenderList__getTextRender);
    circa_patch_function(module, "RenderList.sendCommand", RenderList__sendCommand);
    circa_patch_function(module, "RenderList.getViewportSize", RenderList__getViewportSize);
    circa_finish_native_module(module);
}
