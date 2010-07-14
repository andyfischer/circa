// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>
#include <importing_macros.h>

#include "gl_util.h"
#include "plastic_common_headers.h"

using namespace circa;

namespace gl_shapes {

struct MultipurposeBuffer
{
    static const int staticMemorySize = 5000;
    static GLfloat staticArray[staticMemorySize];
    GLfloat *heapArray;

    MultipurposeBuffer(int size)
    {
        if (size <= staticMemorySize)
            heapArray = NULL;
        else
            heapArray = new GLfloat(size);
    }
    ~MultipurposeBuffer()
    {
        if (heapArray)
            delete heapArray;
    }
    GLfloat* get()
    {
        if (heapArray)
            return heapArray;
        else
            return staticArray;
    }
    GLfloat& operator[] (int index) { return get()[index]; }
};

GLfloat MultipurposeBuffer::staticArray[MultipurposeBuffer::staticMemorySize];

GLint current_program = 0;

void set_gl_color(TaggedValue* color)
{
    glColor4f(color->getIndex(0)->asFloat(),
              color->getIndex(1)->asFloat(),
              color->getIndex(2)->asFloat(),
              color->getIndex(3)->asFloat());
}

void clear_gl_color()
{
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void load_2d_vector_list_to_buffer(TaggedValue* list, GLfloat* buffer)
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

CA_FUNCTION(background)
{
    gl_clear_error();

    TaggedValue* color = INPUT(0);

    glClearColor(color->getIndex(0)->asFloat(),
            color->getIndex(1)->asFloat(),
            color->getIndex(2)->asFloat(),
            color->getIndex(3)->asFloat());

    glClear(GL_COLOR_BUFFER_BIT);

    gl_check_error(CONTEXT_AND_CALLER);
}

CA_FUNCTION(gl_triangles)
{
    TaggedValue* list = INPUT(0);
    TaggedValue* color = INPUT(1);

    set_gl_color(color);
    glBindTexture(GL_TEXTURE_2D, 0);

    int numElements = list->numElements();
    MultipurposeBuffer buffer(numElements * 3);

    load_2d_vector_list_to_buffer(list, buffer.get());

    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT_AND_CALLER);
}

CA_FUNCTION(gl_line_strip)
{
    TaggedValue* list = INPUT(0);
    TaggedValue* color = INPUT(1);

    set_gl_color(color);

    int numElements = list->numElements();
    MultipurposeBuffer buffer(numElements * 3);

    load_2d_vector_list_to_buffer(list, buffer.get());
    
    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT_AND_CALLER);
}

CA_FUNCTION(gl_line_loop)
{
    TaggedValue* list = INPUT(0);
    TaggedValue* color = INPUT(1);

    set_gl_color(color);

    int numElements = list->numElements();
    MultipurposeBuffer buffer(numElements * 3);

    load_2d_vector_list_to_buffer(list, buffer.get());
    
    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_LOOP, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT_AND_CALLER);
}

CA_FUNCTION(gl_lines)
{
    TaggedValue* list = INPUT(0);
    TaggedValue* color = INPUT(1);

    set_gl_color(color);

    int numElements = list->numElements();
    MultipurposeBuffer buffer(numElements * 3);

    load_2d_vector_list_to_buffer(list, buffer.get());
    
    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_LOOP, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT_AND_CALLER);
}

CA_FUNCTION(gl_points)
{
    TaggedValue* list = INPUT(0);
    TaggedValue* color = INPUT(1);

    set_gl_color(color);

    int numElements = list->numElements();
    MultipurposeBuffer buffer(numElements * 3);

    load_2d_vector_list_to_buffer(list, buffer.get());

    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_POINTS, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT_AND_CALLER);
}

CA_FUNCTION(gl_circle)
{
    float x = 0;
    float y = 0;
    circa::point_t::read(INPUT(0), &x, &y);
    float radius = FLOAT_INPUT(1);
    TaggedValue* color = INPUT(2);
    set_gl_color(color);

    // Dumb guess on how many polygons to use
    int control_points = int(radius/3) + 10;
    if (control_points < 15) control_points = 15;

    MultipurposeBuffer buffer(control_points * 3);
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

    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 1 + control_points*2);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT_AND_CALLER);
}

CA_FUNCTION(gl_pie)
{
    float x = 0;
    float y = 0;
    circa::point_t::read(INPUT(0), &x, &y);
    float radius = FLOAT_INPUT(1);
    float angle_start = FLOAT_INPUT(2);
    float angle_fin = FLOAT_INPUT(3);
    TaggedValue* color = INPUT(4);
    set_gl_color(color);

    if (angle_start > angle_fin) {
        float swap = angle_start; angle_start = angle_fin; angle_fin = swap;
    }
    float angle_span = angle_fin - angle_start;

    // Dumb guess on how many polygons to use
    int control_points = int(radius/3) + 10;
    if (control_points < 15) control_points = 15;

    MultipurposeBuffer buffer(control_points * 3);
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

    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLE_FAN, 0, control_points*2 + 1);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT_AND_CALLER);
}

CA_FUNCTION(load_program)
{
    std::string vertFilename =
        get_path_relative_to_source(CALLER, INPUT(0)->asString());
    std::string fragFilename =
        get_path_relative_to_source(CALLER, INPUT(1)->asString());

    if (!file_exists(vertFilename)) {
        error_occurred(CONTEXT_AND_CALLER, "File not found: " + vertFilename);
        return;
    }
    if (!file_exists(fragFilename)) {
        error_occurred(CONTEXT_AND_CALLER, "File not found: " + fragFilename);
        return;
    }

    std::string vertContents = read_text_file(vertFilename);
    const char* vertContentsCStr = vertContents.c_str();

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, (const GLchar**) &vertContentsCStr, 0);
    glCompileShader(vertShader);

    GLint success;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar error[256];
        glGetShaderInfoLog(vertShader, sizeof(error), 0, error);
        error_occurred(CONTEXT_AND_CALLER, error);
        return;
    }

    std::string fragContents = read_text_file(fragFilename);
    const char* fragContentsCStr = fragContents.c_str();

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, (const GLchar**) &fragContentsCStr, 0);
    glCompileShader(fragShader);

    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar error[256];
        glGetShaderInfoLog(fragShader, sizeof(error), 0, error);
        error_occurred(CONTEXT_AND_CALLER, error);
        return;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar error[256];
        glGetProgramInfoLog(program, sizeof(error), 0, error);
        error_occurred(CONTEXT_AND_CALLER, error);
        return;
    }

    gl_check_error(CONTEXT_AND_CALLER);

    set_int(OUTPUT, program);
}

CA_FUNCTION(use_program)
{
    current_program = INT_INPUT(0);
    glUseProgram(current_program);
    gl_check_error(CONTEXT_AND_CALLER);
}

CA_FUNCTION(set_uniform)
{
    const char* name = STRING_INPUT(0);
    TaggedValue* input = INPUT(1);

    GLint loc = glGetUniformLocation(current_program, name);

    if (gl_check_error(CONTEXT_AND_CALLER))
        return;

    if (is_int(input))
        glUniform1i(loc, as_int(input));
    else if (is_float(input))
        glUniform1f(loc, as_float(input));
    else if (is_branch(input)) {

        Branch& contents = as_branch(input);
        int list_length = contents.length();

        if (contents[0]->type == FLOAT_TYPE) {
            float* values = new float[list_length];

            for (int i=0; i < list_length; i++)
                values[i] = to_float(contents[i]);
            glUniform1fv(loc, list_length, values);
            delete[] values;
        } else if (is_branch(contents[0])) {
            int item_length = contents[0]->asBranch().length();

            if (item_length != 2) {
                error_occurred(CONTEXT_AND_CALLER, "Unsupported item length");
                return;
            }

            float* values = new float[list_length*item_length];

            int write = 0;

            for (int i=0; i < list_length; i++)
                for (int j=0; j < item_length; j++)
                    values[write++] = contents[i]->asBranch()[j]->toFloat();

            glUniform2fv(loc, write, values);

            delete[] values;
        }

    } else {
        error_occurred(CONTEXT_AND_CALLER, "Unsupported type: " + input->value_type->name);
        return;
    }

    gl_check_error(CONTEXT_AND_CALLER);
}

void setup(Branch& branch)
{
    install_function(branch["background"], background);
    Branch& gl_ns = branch["gl"]->asBranch();
    install_function(gl_ns["triangles"], gl_triangles);
    install_function(gl_ns["line_strip"], gl_line_strip);
    install_function(gl_ns["line_loop"], gl_line_loop);
    install_function(gl_ns["lines"], gl_lines);
    install_function(gl_ns["points"], gl_points);
    install_function(gl_ns["circle"], gl_circle);
    install_function(gl_ns["pie"], gl_pie);
    install_function(gl_ns["load_program"], load_program);
    install_function(gl_ns["_use_program"], use_program);
    install_function(gl_ns["set_uniform"], set_uniform);
}

} // namespace gl_shapes
