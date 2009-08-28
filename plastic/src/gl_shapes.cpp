// Copyright 2008 Andrew Fischer

#include <SDL_opengl.h>

#include <circa.h>

using namespace circa;

namespace gl_shapes {

void _unpack_gl_color(int color)
{
    float r = ((color & 0xff000000) >> 24) / 255.0f;
    float g = ((color & 0x00ff0000) >> 16) / 255.0f;
    float b = ((color & 0x0000ff00) >> 8) / 255.0f;
    float a = ((color & 0x000000ff) >> 0) / 255.0f;
    glColor4f(r,g,b,a);
}

void background(Term* caller)
{
    int color = caller->input(0)->asInt();

    glClearColor(((color & 0xff000000) >> 24) / 255.0f,
              ((color & 0xff0000) >> 16) / 255.0f,
              ((color & 0xff00) >> 8) / 255.0f,
              (color & 0xff) / 255.0f);

    glClear(GL_COLOR_BUFFER_BIT);
}

void gl_triangles(Term* caller)
{
    Branch& list = caller->input(0)->asBranch();
    int color = caller->input(1)->asInt();

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
    int color = caller->input(1)->asInt();

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
    int color = caller->input(1)->asInt();

    _unpack_gl_color(color);

    glBegin(GL_LINE_LOOP);

    for (int i=0; i < list.length(); i++) {
        float x = list[i]->asBranch()[0]->toFloat();
        float y = list[i]->asBranch()[1]->toFloat();
        glVertex3f(x,y,0);
    }
    
    glEnd();
}

void initialize(Branch& branch)
{
    import_function(branch, background, "background(int)");

    Branch& gl_ns = create_namespace(branch, "gl");
    import_function(gl_ns, gl_triangles, "triangles(List, int)");
    import_function(gl_ns, gl_line_strip, "line_strip(List, int)");
    import_function(gl_ns, gl_line_loop, "line_loop(List, int)");
}

} // namespace gl_shapes
