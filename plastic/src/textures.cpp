// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "plastic_common_headers.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>

#include <circa.h>
#include <importing_macros.h>

#include "gl_util.h"
#include "image.h"
#include "textures.h"

using namespace circa;

namespace textures {

CA_FUNCTION(hosted_load_texture)
{
    Int texid = INPUT(0);

    if (texid == 0) {
        std::string filename = INPUT(1)->asString();
        GLuint id = load_image_to_texture(CONTEXT, CALLER, filename.c_str());
        texid = id;
    }
    set_int(OUTPUT, texid);

    gl_check_error(CONTEXT, CALLER);
}

CA_FUNCTION(hosted_image)
{
    Int texid = INPUT(0);
    std::string filename = INPUT(1)->asString();
    float x1 = FLOAT_INPUT(2);
    float y1 = FLOAT_INPUT(3);
    float x2 = FLOAT_INPUT(4);
    float y2 = FLOAT_INPUT(5);

    if (texid == 0) {
        texid = load_image_to_texture(CONTEXT, CALLER, filename.c_str());
        if (CONTEXT->errorOccurred) return;
    }

    glBindTexture(GL_TEXTURE_2D, texid);

    glBegin(GL_QUADS);

    glTexCoord2d(0.0, 0.0);
    glVertex3f(x1, y1, 0);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(x2, y1,0);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(x2, y2,0);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(x1, y2,0);

    glEnd();

    // reset state
    glBindTexture(GL_TEXTURE_2D, 0);

    gl_check_error(CONTEXT, CALLER);
}

void setup(circa::Branch& branch)
{
    install_function(branch["load_texture"], hosted_load_texture);
    install_function(branch["draw_image"], hosted_image);
}

} // namespace textures
