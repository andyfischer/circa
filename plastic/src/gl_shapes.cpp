// Copyright 2008 Paul Hodge

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
        glVertex3f(x + .5, y + .5, 0);
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
}

} // namespace gl_shapes
