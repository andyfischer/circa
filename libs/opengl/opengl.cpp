// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "types/rect.h"

using namespace circa;

namespace opengl_support {

CA_FUNCTION(new_texture_handle)
{
    // TODO: Need to do garbage collection to reclaim IDs
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    set_int(OUTPUT, texture_id);
}

CA_FUNCTION(draw_texture_as_quad)
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

void setup(Branch& kernel)
{
    Branch& ns = nested_contents(kernel["opengl"]);
    install_function(ns["new_texture_handle"], new_texture_handle);
    install_function(ns["draw_texture_as_quad"], draw_texture_as_quad);
}

} // namespace opengl
