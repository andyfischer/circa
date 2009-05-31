// Copyright 2008 Andrew Fischer

#include <SDL_opengl.h>

#include <circa.h>

using namespace circa;

namespace gl_shapes {

void _unpack_gl_color(int color)
{
    glColor4i((color & 0xff000000) >> 24, (color & 0xff0000) >> 16, (color & 0xff00) >> 8, color & 0xff);
}

void background(Term* caller)
{
    int color = caller->input(0)->asInt();

    glClearColor(((color & 0xff000000) >> 24) / 255.0,
              ((color & 0xff0000) >> 16) / 255.0,
              ((color & 0xff00) >> 8) / 255.0,
              (color & 0xff) / 255.0);

    glClear(GL_COLOR_BUFFER_BIT);
}

void shape(Term* caller)
{
    Branch& list = caller->input(0)->asBranch();
    int color = caller->input(1)->asInt();

    _unpack_gl_color(color);

    glBegin(GL_TRIANGLES);
    _unpack_gl_color(color);

    for (int i=0; i < list.length(); i++) {
        float x = list[i]->field(0)->toFloat();
        float y = list[i]->field(1)->toFloat();

        glVertex3f(x,y,0);
    }

    glEnd();
}

void register_functions(Branch& branch)
{
    import_function(branch, background, "background(int)");
    import_function(branch, shape, "shape(List, int)");
}

} // namespace gl_shapes
