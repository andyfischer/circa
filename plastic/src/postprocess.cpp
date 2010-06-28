// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

// With thanks to Philip Rideout for example code. See: http://prideout.net/bloom/index.php

#include "importing_macros.h"
#include "plastic.h"

using namespace circa;

namespace postprocess_functions
{
    namespace surface_t {
        int get_tex_id(TaggedValue* v) { return as_int(v->getIndex(0)); }
        int get_fbo_id(TaggedValue* v) { return as_int(v->getIndex(1)); }
        int get_width(TaggedValue* v) { return as_int(v->getIndex(2)); }
        int get_height(TaggedValue* v) { return as_int(v->getIndex(3)); }

        void set_tex_id(TaggedValue* v, int id) { touch(v); set_int(v->getIndex(0), id); }
        void set_fbo_id(TaggedValue* v, int id) { touch(v); set_int(v->getIndex(1), id); }
        void set_width(TaggedValue* v, int w) { touch(v); set_int(v->getIndex(2), w); }
        void set_height(TaggedValue* v, int h) { touch(v); set_int(v->getIndex(3), h); }
    }
    
    CA_FUNCTION(make_surface)
    {
        gl_clear_error();
        TaggedValue* surface = INPUT(0);

        if (surface_t::get_tex_id(surface) == 0) {
            int desired_width = INT_INPUT(1);
            int desired_height = INT_INPUT(2);

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

            if (gl_check_error(CONTEXT_AND_CALLER))
                return;

            // Create FBO
            int fbo_id;
            glGenFramebuffersEXT(1, (GLuint*) &fbo_id);
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id);
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                    GL_TEXTURE_2D, tex_id, 0);
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            surface_t::set_fbo_id(surface, fbo_id);

            if (gl_check_error(CONTEXT_AND_CALLER))
                return;


            surface_t::set_width(surface, desired_width);
            surface_t::set_width(surface, desired_height);
        }

        copy(INPUT(0), OUTPUT);
    }

    static int bound_surface_width = 0;
    static int bound_surface_height = 0;

    void bind_surface(TaggedValue* surface)
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

    CA_FUNCTION(bind_surface_hosted)
    {
        bind_surface(INPUT(0));
        gl_check_error(CONTEXT_AND_CALLER);
    }

    CA_FUNCTION(draw_surface)
    {
        TaggedValue* source_surface = INPUT(0);
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

        gl_check_error(CONTEXT_AND_CALLER);
    }

    CA_FUNCTION(copy_surface)
    {
        TaggedValue* source_surface = INPUT(0);
        TaggedValue* dest_surface = INPUT(1);

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
