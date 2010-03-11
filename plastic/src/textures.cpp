// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "common_headers.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>

#include <circa.h>

#include "gl_common.h"
#include "image.h"
#include "textures.h"

using namespace circa;

namespace textures {

void hosted_load_texture(EvalContext* cxt, Term* caller)
{
    Int texid = caller->input(0);

    if (texid == 0) {
        std::string filename = caller->input(1)->asString();
        GLuint id = load_image_to_texture(cxt, caller, filename.c_str());
        texid = id;
    }
    set_int(caller, texid);

    gl_check_error(cxt, caller);
}

void hosted_image(EvalContext* cxt, Term* caller)
{
    Int texid = caller->input(0);
    std::string filename = caller->input(1)->asString();
    float x1 = caller->input(2)->toFloat();
    float y1 = caller->input(3)->toFloat();
    float x2 = caller->input(4)->toFloat();
    float y2 = caller->input(5)->toFloat();

    if (texid == 0) {
        texid = load_image_to_texture(cxt, caller, filename.c_str());
        if (caller->hasError()) return;
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

    gl_check_error(cxt, caller);
}

void setup(circa::Branch& branch)
{
    install_function(branch["load_texture"], hosted_load_texture);
    install_function(branch["draw_image"], hosted_image);
}

} // namespace textures
