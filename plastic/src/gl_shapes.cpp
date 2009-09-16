// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include <SDL_opengl.h>

#include <circa.h>

using namespace circa;

namespace gl_shapes {

void _unpack_gl_color(Term* colorTerm)
{
    Branch& color = as_branch(colorTerm);
    glColor4f(color[0]->asFloat(),
                 color[1]->asFloat(),
                 color[2]->asFloat(),
                 color[3]->asFloat());
}

void background(Term* caller)
{
    Branch& color = caller->input(0)->asBranch();

    glClearColor(color[0]->asFloat(),
                 color[1]->asFloat(),
                 color[2]->asFloat(),
                 color[3]->asFloat());

    glClear(GL_COLOR_BUFFER_BIT);
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
}

void gl_circle(Term* caller)
{
    Branch& center = caller->input(0)->asBranch();
    float x = center[0]->toFloat();
    float y = center[1]->toFloat();
    float radius = caller->input(1)->toFloat();
    Term* color = caller->input(2);

    _unpack_gl_color(color);

    int control_points = int(radius);

    glBegin(GL_TRIANGLE_FAN);

    glVertex3f(x,y,0);

    for (int i=0; i <= control_points; i++) {
        float angle_0 = float(i) / control_points * M_PI * 2;
        float angle_1 = float(i+1) / control_points * M_PI * 2;

        glVertex3f(x + radius * std::cos(angle_0), y + radius * std::sin(angle_0), 0);
        glVertex3f(x + radius * std::cos(angle_1), y + radius * std::sin(angle_1), 0);
    }

    glEnd();
}

void setup(Branch& branch)
{
    install_function(branch["background"], background);
    install_function(branch["gl"]->asBranch()["triangles"], gl_triangles);
    install_function(branch["gl"]->asBranch()["line_strip"], gl_line_strip);
    install_function(branch["gl"]->asBranch()["line_loop"], gl_line_loop);
    install_function(branch["gl"]->asBranch()["points"], gl_points);
    install_function(branch["gl"]->asBranch()["circle"], gl_circle);
}

} // namespace gl_shapes
