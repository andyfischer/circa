// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

// With thanks to Philip Rideout for example code. See: http://prideout.net/bloom/index.php

#include "plastic.h"

using namespace circa;

namespace postprocess_functions
{
    namespace surface_t {
        int get_tex_id(Term* term) { return as_int(term->field(0)); }
        int get_fbo_id(Term* term) { return as_int(term->field(1)); }
        int get_width(Term* term) { return as_int(term->field(2)); }
        int get_height(Term* term) { return as_int(term->field(3)); }

        void set_tex_id(Term* term, int id) { set_int(term->field(0), id); }
        void set_fbo_id(Term* term, int id) { set_int(term->field(1), id); }
        void set_width(Term* term, int w) { set_int(term->field(2), w); }
        void set_height(Term* term, int h) { set_int(term->field(3), h); }
    }
    
    void make_surface(EvalContext* cxt, Term* caller)
    {
        gl_clear_error();
        Term* surface = caller->input(0);

        if (surface_t::get_tex_id(surface) == 0) {
            int desired_width = int_input(caller, 1);
            int desired_height = int_input(caller, 2);

            GLenum internalFormat = /*fp ? GL_RGBA16F_ARB :*/ GL_RGBA;
            GLenum type = /*fp ? GL_HALF_FLOAT_ARB :*/ GL_UNSIGNED_BYTE;
            GLenum filter = /*linear ? GL_LINEAR :*/ GL_NEAREST;

            // Create a color texture
            int tex_id;
            glGenTextures(1, (GLuint*) &tex_id);
            glBindTexture(GL_TEXTURE_2D, tex_id);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, desired_width, desired_height,
                    0, GL_RGBA, type, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
            glBindTexture(GL_TEXTURE_2D, 0);
            surface_t::set_tex_id(surface, tex_id);

            if (gl_check_error(cxt, caller))
                return;

            // Create FBO
            int fbo_id;
            glGenFramebuffersEXT(1, (GLuint*) &fbo_id);
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id);
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                    GL_TEXTURE_2D, tex_id, 0);
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            surface_t::set_fbo_id(surface, fbo_id);

            if (gl_check_error(cxt, caller))
                return;


            surface_t::set_width(surface, desired_width);
            surface_t::set_width(surface, desired_height);
        }

        copy(caller->input(0), caller);
    }

    static int bound_surface_width = 0;
    static int bound_surface_height = 0;

    void bind_surface(Term* surface)
    {
        int width = surface_t::get_width(surface);
        int height = surface_t::get_height(surface);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, surface_t::get_fbo_id(surface));
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1000.0f, 1000.0f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        bound_surface_width = width;
        bound_surface_height = height;
    }

    void bind_surface_hosted(EvalContext* cxt, Term* caller)
    {
        bind_surface(caller->input(0));
        gl_check_error(cxt, caller);
    }

    void draw_surface(EvalContext* cxt, Term* caller)
    {
        Term* source_surface = caller->input(0);
        int tex_id = surface_t::get_tex_id(source_surface);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex_id);

        int width = bound_surface_width;
        int height = bound_surface_height;

        glBegin(GL_QUADS);
        glTexCoord2i(0, 1); glVertex2i(0, 0);
        glTexCoord2i(1, 1); glVertex2i(width, 0);
        glTexCoord2i(1, 0); glVertex2i(width, height);
        glTexCoord2i(0, 0); glVertex2i(0, height);
        glEnd();

        // Reset state
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        gl_check_error(cxt, caller);
    }

    void copy_surface(EvalContext* cxt, Term* caller)
    {
        Term* source_surface = caller->input(0);
        Term* dest_surface = caller->input(1);

        bind_surface(dest_surface);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, surface_t::get_tex_id(source_surface));

        int dest_width = surface_t::get_width(dest_surface);
        int dest_height = surface_t::get_height(dest_surface);

        glBegin(GL_QUADS);
        glTexCoord2i(0, 1); glVertex2i(0, 0);
        glTexCoord2i(1, 1); glVertex2i(dest_width, 0);
        glTexCoord2i(1, 0); glVertex2i(dest_width, dest_height);
        glTexCoord2i(0, 0); glVertex2i(0, dest_height);
        glEnd();

        // Reset state
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void setup(Branch& kernel)
    {
        Branch& postprocess_ns = kernel["postprocess"]->asBranch();
        install_function(postprocess_ns["make_surface"], make_surface);
        install_function(postprocess_ns["draw_surface"], draw_surface);
        install_function(postprocess_ns["bind_surface"], bind_surface_hosted);
        install_function(postprocess_ns["copy_surface"], copy_surface);
    }

} // namespace postprocess_functions
