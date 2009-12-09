// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

#include "plastic.h"

using namespace circa;

namespace gl_shapes {

GLint current_program = 0;

void _unpack_gl_color(Term* colorTerm)
{
    Branch& color = as_branch(colorTerm);
    glColor4f(color[0]->asFloat(), color[1]->asFloat(),
                 color[2]->asFloat(), color[3]->asFloat());
}

void background(Term* caller)
{
    gl_clear_error();

    Branch& color = caller->input(0)->asBranch();

    glClearColor(color[0]->asFloat(), color[1]->asFloat(),
            color[2]->asFloat(), color[3]->asFloat());

    glClear(GL_COLOR_BUFFER_BIT);

    gl_check_error(caller);
}

void gl_triangles(Term* caller)
{
    Branch& list = caller->input(0)->asBranch();
    Term* color = caller->input(1);

    _unpack_gl_color(color);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBegin(GL_TRIANGLES);

    for (int i=0; i < list.length(); i++) {
        float x = list[i]->asBranch()[0]->toFloat();
        float y = list[i]->asBranch()[1]->toFloat();

        glVertex3f(x,y,0);
    }

    glEnd();

    gl_check_error(caller);
}

void gl_line_strip(Term* caller)
{
    Branch& list = caller->input(0)->asBranch();
    Term* color = caller->input(1);

    _unpack_gl_color(color);

    glBegin(GL_LINE_STRIP);

    for (int i=0; i < list.length(); i++) {
        float x = list[i]->asBranch()[0]->toFloat();
        float y = list[i]->asBranch()[1]->toFloat();
        glVertex3f(x,y,0);
    }
    
    glEnd();

    gl_check_error(caller);
}

void gl_line_loop(Term* caller)
{
    Branch& list = caller->input(0)->asBranch();
    Term* color = caller->input(1);

    _unpack_gl_color(color);

    glBegin(GL_LINE_LOOP);

    for (int i=0; i < list.length(); i++) {
        float x = list[i]->asBranch()[0]->toFloat();
        float y = list[i]->asBranch()[1]->toFloat();
        glVertex3f(x,y,0);
    }
    
    glEnd();

    gl_check_error(caller);
}

void gl_points(Term* caller)
{
    Branch& list = caller->input(0)->asBranch();
    Term* color = caller->input(1);

    _unpack_gl_color(color);

    glBegin(GL_POINTS);

    for (int i=0; i < list.length(); i++) {
        float x = list[i]->asBranch()[0]->toFloat();
        float y = list[i]->asBranch()[1]->toFloat();
        glVertex3f(x + 0.5f, y + 0.5f, 0);
    }
    
    glEnd();

    gl_check_error(caller);
}

void gl_circle(Term* caller)
{
    Branch& center = caller->input(0)->asBranch();
    float x = center[0]->toFloat();
    float y = center[1]->toFloat();
    float radius = caller->input(1)->toFloat();
    Term* color = caller->input(2);

    _unpack_gl_color(color);

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

    gl_check_error(caller);
}

void load_program(Term* caller)
{
    std::string vertFilename =
        get_path_relative_to_source(caller, caller->input(0)->asString());
    std::string fragFilename =
        get_path_relative_to_source(caller, caller->input(1)->asString());

    if (!file_exists(vertFilename)) {
        error_occurred(caller, "File not found: " + vertFilename);
        return;
    }
    if (!file_exists(fragFilename)) {
        error_occurred(caller, "File not found: " + fragFilename);
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
        error_occurred(caller, error);
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
        error_occurred(caller, error);
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
        error_occurred(caller, error);
        return;
    }

    gl_check_error(caller);

    as_int(caller) = program;
}

void use_program(Term* caller)
{
    current_program = int_input(caller, 0);
    glUseProgram(current_program);
    gl_check_error(caller);
}

void set_uniform(Term* caller)
{
    const char* name = string_input(caller, 0);
    Term* input = caller->input(1);

    GLint loc = glGetUniformLocation(current_program, name);

    if (gl_check_error(caller))
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
                error_occurred(caller, "Unsupported item length");
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
        error_occurred(caller, "Unsupported type: " + input->type->name);
        return;
    }

    gl_check_error(caller);
}

void setup(Branch& branch)
{
    install_function(branch["background"], background);
    Branch& gl_ns = branch["gl"]->asBranch();
    install_function(gl_ns["triangles"], gl_triangles);
    install_function(gl_ns["line_strip"], gl_line_strip);
    install_function(gl_ns["line_loop"], gl_line_loop);
    install_function(gl_ns["points"], gl_points);
    install_function(gl_ns["circle"], gl_circle);
    install_function(gl_ns["load_program"], load_program);
    install_function(gl_ns["_use_program"], use_program);
    install_function(gl_ns["set_uniform"], set_uniform);
}

} // namespace gl_shapes
