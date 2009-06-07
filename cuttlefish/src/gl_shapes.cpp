// Copyright 2008 Paul Hodge

#include <SDL_opengl.h>

#include <circa.h>

using namespace circa;

namespace gl_shapes {

void _unpack_gl_color(int color)
{
    float r = ((color & 0xff000000) >> 24) / 255.0;
    float g = ((color & 0x00ff0000) >> 16) / 255.0;
    float b = ((color & 0x0000ff00) >> 8) / 255.0;
    float a = ((color & 0x000000ff) >> 0) / 255.0;
    glColor4f(r,g,b,a);
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

void gl_triangles(Term* caller)
{
    Branch& list = caller->input(0)->asBranch();
    int color = caller->input(1)->asInt();

    glDisable(GL_TEXTURE);
    glDisable(GL_TEXTURE_2D);

    _unpack_gl_color(color);
    glBegin(GL_TRIANGLES);

    for (int i=0; i < list.length(); i++) {
        float x = list[i]->field(0)->toFloat();
        float y = list[i]->field(1)->toFloat();

        glVertex3f(x,y,0);
    }

    glEnd();
}

void gl_line_strip(Term* caller)
{
    Branch& list = caller->input(0)->asBranch();
    int color = caller->input(1)->asInt();

    _unpack_gl_color(color);

    glBegin(GL_LINE_STRIP);

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
    import_function(branch, gl_triangles, "gl_triangles(List, int)");
    import_function(branch, gl_line_strip, "gl_line_strip(List, int)");
}

} // namespace gl_shapes
