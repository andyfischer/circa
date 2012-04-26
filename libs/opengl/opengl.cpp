// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa.h"
#include "../shared/opengl.h"

using namespace circa;

struct FrameBuffer
{
    int width;
    int height;
    GLuint tex_id;
    GLuint fbo_id;

    FrameBuffer() : tex_id(0), fbo_id(0) {}
    ~FrameBuffer() {
        // TODO cleanup tex_id and fbo_id
    }
};

Type* g_frameBuffer_t;

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

bool gl_check_error(circa::Stack* cxt)
{
    GLenum errcode = glGetError();
    if (errcode == GL_NO_ERROR)
        return false;

    const char* err = gl_to_string(errcode);

    std::string msg("OpenGL reported error: ");
    msg += err;
    circa::raise_error(cxt, msg.c_str());
    return true;
}

void gl_ignore_error()
{
    glGetError();
}

CA_FUNCTION(opengl__draw_quad)
{
    float x1(0), y1(0), x2(0), y2(0);
    get_rect(INPUT(0), &x1, &y1, &x2, &y2);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(x1, y1, 0);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(x2, y1, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(x2, y2, 0);
    glTexCoord2d(0.0, 0.0);
    glVertex3f(x1, y2, 0);
    glEnd();
}

CA_FUNCTION(opengl__new_texture_handle)
{
    // TODO: Need to do garbage collection to reclaim IDs
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    set_int(OUTPUT, texture_id);
}

void write_2d_vector_list_to_buffer(caValue* list, GLfloat* buffer)
{
    int numElements = list->numElements();
    int write = 0;
    for (int i=0; i < numElements; i++) {
        float x = list->getIndex(i)->getIndex(0)->toFloat();
        float y = list->getIndex(i)->getIndex(1)->toFloat();

        buffer[write++] = x;
        buffer[write++] = y;
        buffer[write++] = 0;
    }
}

void set_gl_color(caValue* color)
{
    glColor4f(color->getIndex(0)->toFloat(),
              color->getIndex(1)->toFloat(),
              color->getIndex(2)->toFloat(),
              color->getIndex(3)->toFloat());
}

void clear_gl_color()
{
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

CA_FUNCTION(gl__triangles)
{
    caValue* list = INPUT(0);
    caValue* color = INPUT(1);

    set_gl_color(color);
    glBindTexture(GL_TEXTURE_2D, 0);

    int numElements = list->numElements();
    GLfloat* buffer = (GLfloat*) malloc(sizeof(GLfloat) * numElements * 3);

    write_2d_vector_list_to_buffer(list, buffer);

    glVertexPointer(3, GL_FLOAT, 0, buffer);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT);

    free(buffer);
}

CA_FUNCTION(gl__line_strip)
{
    caValue* list = INPUT(0);
    caValue* color = INPUT(1);

    set_gl_color(color);

    int numElements = list->numElements();
    GLfloat* buffer = (GLfloat*) malloc(sizeof(GLfloat) * numElements * 3);

    write_2d_vector_list_to_buffer(list, buffer);
    
    glVertexPointer(3, GL_FLOAT, 0, buffer);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT);

    free(buffer);
}

CA_FUNCTION(gl__line_loop)
{
    caValue* list = INPUT(0);
    caValue* color = INPUT(1);

    set_gl_color(color);

    int numElements = list->numElements();
    GLfloat* buffer = (GLfloat*) malloc(sizeof(GLfloat) * numElements * 3);

    write_2d_vector_list_to_buffer(list, buffer);
    
    glVertexPointer(3, GL_FLOAT, 0, buffer);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_LOOP, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT);
    free(buffer);
}

CA_FUNCTION(gl__lines)
{
    caValue* list = INPUT(0);
    caValue* color = INPUT(1);

    set_gl_color(color);

    int numElements = list->numElements();
    GLfloat* buffer = (GLfloat*) malloc(sizeof(GLfloat) * numElements * 3);

    write_2d_vector_list_to_buffer(list, buffer);
    
    glVertexPointer(3, GL_FLOAT, 0, buffer);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINES, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT);
    free(buffer);
}

CA_FUNCTION(gl__points)
{
    caValue* list = INPUT(0);
    caValue* color = INPUT(1);

    set_gl_color(color);

    int numElements = list->numElements();
    GLfloat* buffer = (GLfloat*) malloc(sizeof(GLfloat) * numElements * 3);

    write_2d_vector_list_to_buffer(list, buffer);

    glVertexPointer(3, GL_FLOAT, 0, buffer);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_POINTS, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT);
    free(buffer);
}

CA_FUNCTION(gl__circle)
{
    Point* loc = Point::checkCast(INPUT(0));
    float x = loc->getX();
    float y = loc->getY();
    float radius = FLOAT_INPUT(1);
    caValue* color = INPUT(2);
    set_gl_color(color);

    // Dumb guess on how many polygons to use
    int control_points = int(radius/3) + 10;
    if (control_points < 15) control_points = 15;

    GLfloat* buffer = (GLfloat*) malloc(sizeof(GLfloat) * (2 + control_points) * 6);
    int write = 0;

    buffer[write++] = x;
    buffer[write++] = y;
    buffer[write++] = 0;

    for (int i=0; i <= control_points; i++) {
        float angle_0 = float(float(i) / control_points * M_PI * 2);
        float angle_1 = float(float(i+1) / control_points * M_PI * 2);

        buffer[write++] = x + radius * std::cos(angle_0);
        buffer[write++] = y + radius * std::sin(angle_0);
        buffer[write++] = 0;

        buffer[write++] = x + radius * std::cos(angle_1);
        buffer[write++] = y + radius * std::sin(angle_1);
        buffer[write++] = 0;
    }

    glVertexPointer(3, GL_FLOAT, 0, buffer);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 1 + control_points*2);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT);
    free(buffer);
}

CA_FUNCTION(gl__pie)
{
    Point* loc = Point::checkCast(INPUT(0));
    float x = loc->getX();
    float y = loc->getY();
    float radius = FLOAT_INPUT(1);
    float angle_start = FLOAT_INPUT(2);
    float angle_fin = FLOAT_INPUT(3);
    caValue* color = INPUT(4);
    set_gl_color(color);

    if (angle_start > angle_fin) {
        float swap = angle_start; angle_start = angle_fin; angle_fin = swap;
    }
    float angle_span = angle_fin - angle_start;

    // Dumb guess on how many polygons to use
    int control_points = int(radius/3) + 10;
    if (control_points < 15) control_points = 15;

    GLfloat* buffer = (GLfloat*) malloc(sizeof(GLfloat) * (2 + control_points) * 6);
    int write = 0;

    buffer[write++] = x;
    buffer[write++] = y;
    buffer[write++] = 0;

    for (int i=0; i < control_points; i++) {

        float angle_0 = float(float(i) / control_points * angle_span + angle_start);
        float angle_1 = float(float(i+1) / control_points * angle_span + angle_start);

        // Convert from 0..1 to radians
        angle_0 *= M_PI * 2;
        angle_1 *= M_PI * 2;

        // Use (sin,-cos) so that angle 0 starts at the top and increases clockwise.
        buffer[write++] = x + radius * std::sin(angle_0);
        buffer[write++] = y + radius * -std::cos(angle_0);
        buffer[write++] = 0;

        buffer[write++] = x + radius * std::sin(angle_1);
        buffer[write++] = y + radius * -std::cos(angle_1);
        buffer[write++] = 0;
    }

    glVertexPointer(3, GL_FLOAT, 0, buffer);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLE_FAN, 0, control_points*2 + 1);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT);
}

CA_FUNCTION(gl__draw_texture_as_quad)
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

CA_FUNCTION(opengl__activeTexture)
{
    glActiveTexture(GL_TEXTURE0 + INT_INPUT(0));
}

CA_FUNCTION(opengl__bind_texture)
{
    glBindTexture(GL_TEXTURE_2D, INT_INPUT(0));
}

CA_FUNCTION(opengl__load_shader)
{
    std::string vertText = circa::read_text_file_as_str(STRING_INPUT(0));
    std::string fragText = circa::read_text_file_as_str(STRING_INPUT(1));

    const char* vert = vertText.c_str();
    const char* frag = fragText.c_str();
    
    GLint success;

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, (const GLchar**) &vert, 0);
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar buf[2024];
        glGetShaderInfoLog(vertShader, sizeof(buf), 0, buf);
        RAISE_ERROR(std::string("Unable to compile vertex shader: ")+buf);
        set_int(OUTPUT, 0);
    }

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, (const GLchar**) &frag, 0);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar buf[2024];
        glGetShaderInfoLog(fragShader, sizeof(buf), 0, buf);
        RAISE_ERROR(std::string("Unable to compile fragment shader: ")+buf);
        set_int(OUTPUT, 0);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar buf[256];
        glGetProgramInfoLog(program, sizeof(buf), 0, buf);
        RAISE_ERROR("Unable to link shaders.");
        set_int(OUTPUT, 0);
    }

    set_int(OUTPUT, program);
}

CA_FUNCTION(opengl__load_shader_text)
{
    const char* vertText = STRING_INPUT(0);
    const char* fragText = STRING_INPUT(1);
    GLint success;

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, (const GLchar**) &vertText, 0);
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar buf[2024];
        glGetShaderInfoLog(vertShader, sizeof(buf), 0, buf);
        RAISE_ERROR(std::string("Unable to compile vertex shader: ")+buf);
        set_int(OUTPUT, 0);
    }

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, (const GLchar**) &fragText, 0);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar buf[2024];
        glGetShaderInfoLog(fragShader, sizeof(buf), 0, buf);
        RAISE_ERROR(std::string("Unable to compile fragment shader: ")+buf);
        set_int(OUTPUT, 0);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar buf[256];
        glGetProgramInfoLog(program, sizeof(buf), 0, buf);
        RAISE_ERROR("Unable to link shaders.");
        set_int(OUTPUT, 0);
    }

    set_int(OUTPUT, program);
}

CA_FUNCTION(opengl__shader_quad)
{
    glUseProgram(INT_INPUT(0));

    List& uniforms = *as_list(INPUT(2));
    for (int i=0; i < uniforms.length(); i++) {
        if (!is_list(uniforms[i]))
            return RAISE_ERROR("uniform pair isn't a list");
        GLuint uniform = as_int(list_get(uniforms[i], 0));
        caValue* value = list_get(uniforms[i], 1);
        if (is_int(value))
            glUniform1i(uniform, as_int(value));
        else if (is_float(value))
            glUniform1f(uniform, as_float(value));
        else
            return RAISE_ERROR("unrecognized type as a uniform value");
    }

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
    gl_check_error(CONTEXT);
}

CA_FUNCTION(opengl__get_uniform_location)
{
//std::cout << "name: " << STRING_INPUT(1) << std::endl;
    set_int(OUTPUT, glGetUniformLocation(INT_INPUT(0), STRING_INPUT(1)));
//std::cout << "result: " << OUTPUT->toString() << std::endl;
    gl_check_error(CONTEXT);
}
CA_FUNCTION(opengl__uniform)
{
    GLuint uniform = INT_INPUT(0);
    caValue* value = INPUT(1);
    if (is_int(value))
        glUniform1i(uniform, as_int(value));
    else if (is_float(value))
        glUniform1f(uniform, as_float(value));
    else
        return RAISE_ERROR("unrecognized type as a uniform value");

    gl_check_error(CONTEXT);
}
CA_FUNCTION(opengl__use_program)
{
    glUseProgram(INT_INPUT(0));
    gl_check_error(CONTEXT);
}

#if 0
CA_FUNCTION(set_uniform)
{
    const char* name = STRING_INPUT(0);
    caValue* input = INPUT(1);

    GLint loc = glGetUniformLocation(current_program, name);

    if (gl_check_error(CONTEXT))
        return;

    if (is_int(input)) {
        glUniform1i(loc, as_int(input));
    }
    else if (is_float(input)) {
        glUniform1f(loc, as_float(input));
    }
    else {
        raise_error(CONTEXT, CALLER, "Unsupported type: " + input->value_type->name)
        return;
    }

    gl_check_error(CONTEXT);
}
#endif

CA_FUNCTION(opengl__generate_frame_buffer)
{
    gl_ignore_error();

    int width = to_int(list_get(INPUT(0), 0));
    int height = to_int(list_get(INPUT(0), 1));

    GLenum internalFormat = /*fp ? GL_RGBA16F_ARB :*/ GL_RGBA;
    GLenum type = /*fp ? GL_HALF_FLOAT_ARB :*/ GL_UNSIGNED_BYTE;
    GLenum filter = /*linear ? GL_LINEAR :*/ GL_NEAREST;

    // Create a color texture
    GLuint tex_id(0);
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height,
            0, GL_RGBA, type, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (gl_check_error(CONTEXT))
        return;

    // Create FBO
    GLuint fbo_id(0);
    glGenFramebuffersEXT(1, &fbo_id);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
            GL_TEXTURE_2D, tex_id, 0);

    // Initialize GL state
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1000.0f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gl_check_error(CONTEXT);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    if (gl_check_error(CONTEXT))
        return;

    FrameBuffer* buffer = handle_t::create<FrameBuffer>(OUTPUT, g_frameBuffer_t);
    buffer->width = width;
    buffer->height = height;
    buffer->tex_id = tex_id;
    buffer->fbo_id = fbo_id;
}

CA_FUNCTION(opengl__FrameBuffer_bind)
{
    gl_ignore_error();

    FrameBuffer* buffer = (FrameBuffer*) handle_t::get_ptr(INPUT(0));

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer->fbo_id);

    if (gl_check_error(CONTEXT)) {
        RAISE_ERROR("glBindFramebufferEXT failed");
        return;
    }
}

CA_FUNCTION(opengl__FrameBuffer_draw_quad)
{
    FrameBuffer* buffer = (FrameBuffer*) handle_t::get_ptr(INPUT(0));

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, buffer->tex_id);

    glUniform1i(INT_INPUT(2), buffer->tex_id);

    float x1(0), y1(0), x2(0), y2(0);
    get_rect(INPUT(1), &x1, &y1, &x2, &y2);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(x1, y1, 0);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(x2, y1, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(x2, y2, 0);
    glTexCoord2d(0.0, 0.0);
    glVertex3f(x1, y2, 0);
    glEnd();

    // Cleanup
    glBindTexture(GL_TEXTURE_2D, 0);

    gl_check_error(CONTEXT);
}
CA_FUNCTION(opengl__FrameBuffer_get_tex_id)
{
    FrameBuffer* buffer = (FrameBuffer*) handle_t::get_ptr(INPUT(0));
    set_int(OUTPUT, buffer->tex_id);
}

CA_FUNCTION(opengl__bind_main_frame_buffer)
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void on_load(Branch* branch)
{
    g_frameBuffer_t = setup_type_as_handle<FrameBuffer>(branch, "opengl:FrameBuffer");
}

} // extern "C"
