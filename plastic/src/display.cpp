// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

#include "plastic_common_headers.h"

#include "app.h"
#include "gl_util.h"
#include "plastic_main.h"

#include "display.h"

using namespace circa;

SDL_Surface* SCREEN = NULL;
const int SCREEN_BPP = 32;

bool initialize_display()
{
    // Initialize the window
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Create the surface
    int desiredWidth = 800;
    int desiredHeight = 600;

    if (app::users_branch().contains("desired_window_size")) {
        desiredWidth = app::users_branch()["desired_window_size"]->asBranch()[0]->asInt();
        desiredHeight = app::users_branch()["desired_window_size"]->asBranch()[1]->asInt();
    }

    if (!resize_display(desiredWidth, desiredHeight))
        return false;

#ifdef WINDOWS
    GLenum err = glewInit();

    if (err != GLEW_OK) {
        std::cerr << "glewInit failed: " << glewGetErrorString(err) << std::endl;
        return false;
    }
#endif

    SDL_EnableUNICODE(1);

    return true;
}

bool resize_display(int width, int height)
{
    app::update_window_dimensions(width, height);

    SCREEN = SDL_SetVideoMode(width, height, 16,
            SDL_OPENGL | SDL_SWSURFACE | SDL_RESIZABLE);
            //SDL_OPENGL | SDL_SWSURFACE | SDL_FULLSCREEN);

    if (SCREEN == NULL) {
        std::cerr << "SDL_SetVideoMode failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize desired SDL subsystems
    if (SDL_Init(SDL_INIT_TIMER & SDL_INIT_VIDEO
                & SDL_INIT_JOYSTICK & SDL_INIT_EVENTTHREAD) == -1) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Write window width & height
    set_int(app::runtime_branch()["window"]->getField("width"), width);
    set_int(app::runtime_branch()["window"]->getField("height"), height);

    // Set window caption
    std::string windowTitle;
    if (app::users_branch().contains("desired_window_title"))
        windowTitle = app::users_branch()["desired_window_title"]->asString();
    else
        windowTitle = get_branch_source_filename(app::users_branch());

    SDL_WM_SetCaption(windowTitle.c_str(), NULL);

    // Initialize GL state
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT, GL_FILL);
    glDisable(GL_CULL_FACE);
    glClearColor(0,0,0,0);
    glClearDepth(1000);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, width, height);
     
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1000.0f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT);

    const char* err = gl_check_error();
    if (err != NULL) {
        std::cerr << "GL error during initialization: " << err << std::endl;
        return false;
    }

    return true;
}

void render_frame()
{
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(0);

    app::evaluate_main_script();

    // Check for uncaught GL error
    const char* err = gl_check_error();
    if (err != NULL) {
        std::cerr << "Uncaught GL error (in render_frame): " << err << std::endl;
    }

    // Update the screen
    SDL_GL_SwapBuffers();
}

