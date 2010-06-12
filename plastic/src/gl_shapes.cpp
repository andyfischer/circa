// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

#include "plastic.h"

using namespace circa;

namespace gl_shapes {

GLint current_program = 0;

void _unpack_gl_color(TaggedValue* color)
{
    glColor4f(color->getIndex(0)->asFloat(),
              color->getIndex(1)->asFloat(),
              color->getIndex(2)->asFloat(),
              color->getIndex(3)->asFloat());
}

void background(EvalContext* cxt, Term* caller)
{
    gl_clear_error();

    Term* color = caller->input(0);

    glClearColor(color->getIndex(0)->asFloat(),
            color->getIndex(1)->asFloat(),
            color->getIndex(2)->asFloat(),
            color->getIndex(3)->asFloat());

    glClear(GL_COLOR_BUFFER_BIT);

    gl_check_error(cxt, caller);
}

void gl_triangles(EvalContext* cxt, Term* caller)
{
    Term* list = caller->input(0);
    Term* color = caller->input(1);

    _unpack_gl_color(color);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBegin(GL_TRIANGLES);

    int numElements = list->numElements();
    for (int i=0; i < numElements; i++) {
        float x = list->getIndex(i)->getIndex(0)->toFloat();
        float y = list->getIndex(i)->getIndex(1)->toFloat();

        glVertex3f(x,y,0);
    }

    glEnd();

    gl_check_error(cxt, caller);
}

void gl_line_strip(EvalContext* cxt, Term* caller)
{
    Term* list = caller->input(0);
    Term* color = caller->input(1);

    _unpack_gl_color(color);

    glBegin(GL_LINE_STRIP);

    int numElements = list->numElements();
    for (int i=0; i < numElements; i++) {
        float x = list->getIndex(i)->getIndex(0)->toFloat();
        float y = list->getIndex(i)->getIndex(1)->toFloat();
        glVertex3f(x,y,0);
    }
    
    glEnd();

    gl_check_error(cxt, caller);
}

void gl_line_loop(EvalContext* cxt, Term* caller)
{
    Term* list = caller->input(0);
    Term* color = caller->input(1);

    _unpack_gl_color(color);

    glBegin(GL_LINE_LOOP);

    int numElements = list->numElements();
    for (int i=0; i < numElements; i++) {
        float x = list->getIndex(i)->getIndex(0)->toFloat();
        float y = list->getIndex(i)->getIndex(1)->toFloat();
        glVertex3f(x,y,0);
    }
    
    glEnd();

    gl_check_error(cxt, caller);
}

void gl_lines(EvalContext* cxt, Term* caller)
{
    Term* list = caller->input(0);
    Term* color = caller->input(1);

    _unpack_gl_color(color);

    glBegin(GL_LINES);

    int numElements = list->numElements();
    for (int i=0; i < numElements; i++) {
        float x = list->getIndex(i)->getIndex(0)->toFloat();
        float y = list->getIndex(i)->getIndex(1)->toFloat();
        glVertex3f(x,y,0);
    }
    
    glEnd();

    gl_check_error(cxt, caller);
}

void gl_points(EvalContext* cxt, Term* caller)
{
    Term* list = caller->input(0);
    Term* color = caller->input(1);

    _unpack_gl_color(color);

    glBegin(GL_POINTS);

    int numElements = list->numElements();
    for (int i=0; i < numElements; i++) {
        float x = list->getIndex(i)->getIndex(0)->toFloat();
        float y = list->getIndex(i)->getIndex(1)->toFloat();
        glVertex3f(x + 0.5f, y + 0.5f, 0);
    }
    
    glEnd();

    gl_check_error(cxt, caller);
}

void gl_circle(EvalContext* cxt, Term* caller)
{
    float x = 0;
    float y = 0;
    circa::point_t::read(caller->input(0), &x, &y);
    float radius = caller->input(1)->toFloat();
    Term* color = caller->input(2);
    _unpack_gl_color(color);

    // Dumb guess on how many polygons to use
    int control_points = int(radius/3) + 10;
    if (control_points < 15) control_points = 15;

    glBegin(GL_TRIANGLE_FAN);

    glVertex3f(x,y,0);

    for (int i=0; i <= control_points; i++) {
        float angle_0 = float(float(i) / control_points * M_PI * 2);
        float angle_1 = float(float(i+1) / control_points * M_PI * 2);

        glVertex3f(x + radius * std::cos(angle_0), y + radius * std::sin(angle_0), 0);
        glVertex3f(x + radius * std::cos(angle_1), y + radius * std::sin(angle_1), 0);
    }

    glEnd();

    gl_check_error(cxt, caller);
}

void gl_pie(EvalContext* cxt, Term* caller)
{
    float x = 0;
    float y = 0;
    circa::point_t::read(caller->input(0), &x, &y);
    float radius = float_input(caller, 1);
    float angle_start = float_input(caller, 2);
    float angle_fin = float_input(caller, 3);
    Term* color = caller->input(4);
    _unpack_gl_color(color);

    if (angle_start > angle_fin) {
        float swap = angle_start; angle_start = angle_fin; angle_fin = swap;
    }
    if (angle_start < 0) angle_start = 0;
    if (angle_fin > 1) angle_fin = 1;

    float angle_span = angle_fin - angle_start;

    // Dumb guess on how many polygons to use
    int control_points = 15;

    glBegin(GL_TRIANGLE_FAN);

    glVertex3f(x,y,0);

    for (int i=0; i <= control_points; i++) {
        float angle_0 = float(float(i) / control_points * angle_span + angle_start);
        float angle_1 = float(float(i+1) / control_points * angle_span + angle_start);

        // Convert from 0..1 to radians
        angle_0 *= M_PI * 2;
        angle_1 *= M_PI * 2;
        
        // Use (sin,-cos) so that angle 0 starts at the top and increases clockwise.
        glVertex3f(x + radius * std::sin(angle_0), y + radius * -std::cos(angle_0), 0);
        glVertex3f(x + radius * std::sin(angle_1), y + radius * -std::cos(angle_1), 0);
    }

    glEnd();
}

void load_program(EvalContext* cxt, Term* caller)
{
    std::string vertFilename =
        get_path_relative_to_source(caller, caller->input(0)->asString());
    std::string fragFilename =
        get_path_relative_to_source(caller, caller->input(1)->asString());

    if (!file_exists(vertFilename)) {
        error_occurred(cxt, caller, "File not found: " + vertFilename);
        return;
    }
    if (!file_exists(fragFilename)) {
        error_occurred(cxt, caller, "File not found: " + fragFilename);
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
        error_occurred(cxt, caller, error);
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
        error_occurred(cxt, caller, error);
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
        error_occurred(cxt, caller, error);
        return;
    }

    gl_check_error(cxt, caller);

    set_int(caller, program);
}

void use_program(EvalContext* cxt, Term* caller)
{
    current_program = int_input(caller, 0);
    glUseProgram(current_program);
    gl_check_error(cxt, caller);
}

void set_uniform(EvalContext* cxt, Term* caller)
{
    const char* name = string_input(caller, 0);
    Term* input = caller->input(1);

    GLint loc = glGetUniformLocation(current_program, name);

    if (gl_check_error(cxt, caller))
        return;

    if (input->type == INT_TYPE)
        glUniform1i(loc, as_int(input));
    else if (input->type == FLOAT_TYPE)
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
                error_occurred(cxt, caller, "Unsupported item length");
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
        error_occurred(cxt, caller, "Unsupported type: " + input->type->name);
        return;
    }

    gl_check_error(cxt, caller);
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
