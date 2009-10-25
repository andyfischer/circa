// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

#include "plastic.h"

using namespace circa;

SDL_Surface* SCREEN = NULL;
const int SCREEN_BPP = 32;
int WINDOW_WIDTH = 0;
int WINDOW_HEIGHT = 0;

bool initialize_display()
{
    // Initialize the window
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Create the surface
    WINDOW_WIDTH = 640;
    WINDOW_HEIGHT = 480;

    if (users_branch().contains("desired_window_size")) {
        WINDOW_WIDTH = users_branch()["desired_window_size"]->asBranch()[0]->asInt();
        WINDOW_HEIGHT = users_branch()["desired_window_size"]->asBranch()[1]->asInt();
    }

    SCREEN = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 16, SDL_OPENGL | SDL_SWSURFACE);

    if (SCREEN == NULL) {
        std::cerr << "SDL_SetVideoMode failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Write window width & height to runtime.ca
    runtime_branch()["window"]->asBranch()["width"]->asInt() = WINDOW_WIDTH;
    runtime_branch()["window"]->asBranch()["height"]->asInt() = WINDOW_HEIGHT;

    // Initialize desired SDL subsystems
    if (SDL_Init(SDL_INIT_TIMER & SDL_INIT_VIDEO
                & SDL_INIT_JOYSTICK & SDL_INIT_EVENTTHREAD) == -1) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set window caption
    std::string windowTitle;
    if (users_branch().contains("desired_window_title"))
        windowTitle = users_branch()["desired_window_title"]->asString();
    else
        windowTitle = get_branch_source_filename(users_branch());

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

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
     
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1000.0f, 1000.0f);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glClear( GL_COLOR_BUFFER_BIT );

    Term errorListener;
    gl_check_error(&errorListener, " (initialize display)");

    if (errorListener.hasError()) {
        std::cerr << "GL error during initialization: "
            << get_runtime_error_message(&errorListener) << std::endl;
        return false;
    }

    return true;
}

void render_frame()
{
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(0);

    Term errorListener;
    evaluate_branch(runtime_branch(), &errorListener);

    if (errorListener.hasError()) {
        std::cout << "Runtime error:" << std::endl;
        print_runtime_error_formatted(runtime_branch(), std::cout);
        std::cout << std::endl;
        PAUSED = true;
        PAUSE_REASON = RUNTIME_ERROR;
        clear_error(&errorListener);
    }

    // Check for uncaught GL error
    gl_check_error(&errorListener, " (uncaught)");
    if (errorListener.hasError()) {
        std::cout << get_runtime_error_message(&errorListener) << std::endl;
    }

    // Update the screen
    SDL_GL_SwapBuffers();
}

