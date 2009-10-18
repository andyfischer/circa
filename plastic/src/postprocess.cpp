// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

// With thanks to Philip Rideout for example code. See: http://prideout.net/bloom/index.php

#include "plastic.h"

using namespace circa;

namespace postprocess_functions
{
    struct Surface
    {
        int& tex_id;
        int& fbo_id;
        int& width;
        int& height;

        Surface(Term* term) :
            tex_id(as_branch(term)[0]->asInt()),
            fbo_id(as_branch(term)[1]->asInt()),
            width(as_branch(term)[2]->asInt()),
            height(as_branch(term)[3]->asInt())
        {
        }
    };
    
    void hidden_surface(Term* caller)
    {
        gl_clear_error();

        Surface surface = Surface(caller->input(0));

        if (surface.tex_id == 0) {
            int desired_width = int_input(caller, 1);
            int desired_height = int_input(caller, 2);

            GLenum internalFormat = /*fp ? GL_RGBA16F_ARB :*/ GL_RGBA;
            GLenum type = /*fp ? GL_HALF_FLOAT_ARB :*/ GL_UNSIGNED_BYTE;
            GLenum filter = /*linear ? GL_LINEAR :*/ GL_NEAREST;

            // Create a color texture
            glGenTextures(1, (GLuint*) &surface.tex_id);
            glBindTexture(GL_TEXTURE_2D, surface.tex_id);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, desired_width, desired_height,
                    0, GL_RGBA, type, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
            glBindTexture(GL_TEXTURE_2D, 0);

            if (gl_check_error(caller))
                return;

            // Create FBO
            glGenFramebuffersEXT(1, (GLuint*) &surface.fbo_id);
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, surface.fbo_id);
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                    GL_TEXTURE_2D, surface.tex_id, 0);
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

            if (gl_check_error(caller))
                return;

            surface.width = desired_width;
            surface.height = desired_height;
        }

        assign_value(caller->input(0), caller);
    }

    void get_tex_id_for_main_surface(Term* caller)
    {
        int& tex_id = as_int(caller->input(0));

        if (tex_id == 0) {
            glGenTextures(1, (GLuint*) &tex_id);
            glBindTexture(GL_TEXTURE_2D, tex_id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                    WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);

            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                    GL_TEXTURE_2D, tex_id, 0);
        }
        as_int(caller) = tex_id;
    }

    void copy_surface(Term* caller)
    {
        Surface source_surface(caller->input(0));
        Surface dest_surface(caller->input(1));
        float alpha = float_input(caller, 2);

        // bind surface
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, dest_surface.fbo_id);
        //glViewport(0, 0, dest_surface.width, dest_surface.height);
        //glMatrixMode(GL_PROJECTION);
        //glOrtho(0, dest_surface.width, dest_surface.height, 0, -1000.0f, 1000.0f);
        //glMatrixMode(GL_MODELVIEW);
        //glLoadIdentity();

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, dest_surface.tex_id);

        glColor4f(1,1,1,alpha);
        
        glBegin(GL_QUADS);
        glTexCoord2i(0, 0); glVertex2i(-1, -1);
        glTexCoord2i(1, 0); glVertex2i(1, -1);
        glTexCoord2i(1, 1); glVertex2i(1, 1);
        glTexCoord2i(0, 1); glVertex2i(-1, 1);
        glEnd();

        gl_check_error(caller);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        //glMatrixMode(GL_PROJECTION);
        //glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1000.0f, 1000.0f);
        //glMatrixMode(GL_MODELVIEW);
        //glLoadIdentity();
        glColor4f(1,1,1,1);
    }

    void setup(Branch& kernel)
    {
        Branch& postprocess_ns = kernel["postprocess"]->asBranch();
        install_function(postprocess_ns["hidden_surface"], hidden_surface);
        install_function(postprocess_ns["get_tex_id_for_main_surface"],
                get_tex_id_for_main_surface);
        install_function(postprocess_ns["copy_surface"], copy_surface);
    }

} // namespace postprocess_functions
