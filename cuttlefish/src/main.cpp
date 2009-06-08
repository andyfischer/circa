// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include <SDL.h>
#include <SDL_opengl.h>

#include <circa.h>

#include "gl_shapes.h"
#include "input.h"
#include "mesh.h"
#include "shaders.h"
#include "textures.h"

using namespace circa;

int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

SDL_Surface* SCREEN = NULL;
Branch* SCRIPT_ROOT = NULL;
Branch* USERS_BRANCH = NULL;
bool CONTINUE_MAIN_LOOP = true;

float TIME = 0;
float TIME_DELTA = 0;
long PREV_SDL_TICKS = 0;

bool initialize_display()
{
    // Initialize the window
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Create the surface
    circa::Int windowWidth(*USERS_BRANCH, "WINDOW_WIDTH", 640);
    circa::Int windowHeight(*USERS_BRANCH, "WINDOW_HEIGHT", 480);
    SCREEN = SDL_SetVideoMode(windowWidth, windowHeight, 16, SDL_OPENGL | SDL_SWSURFACE);

    if (SCREEN == NULL) {
        std::cerr << "SDL_SetVideoMode failed: " << std::endl;
        return false;
    }

    // Initialize desired SDL subsystems
    if (SDL_Init(SDL_INIT_TIMER & SDL_INIT_VIDEO & SDL_INIT_JOYSTICK & SDL_INIT_EVENTTHREAD) == -1)
        return false;

    // Set window caption
    SDL_WM_SetCaption(String(*USERS_BRANCH, "WINDOW_TITLE", "Untitled").get().c_str(), NULL);

    // Initialize GL state

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glClearColor(0,0,0,0);
    glClearDepth(1000);
    glDepthFunc(GL_LEQUAL);
     
    glViewport(0, 0, windowWidth, windowHeight);
     
    glClear( GL_COLOR_BUFFER_BIT );
     
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
     
    glOrtho(0, windowWidth, windowHeight, 0, -1000.0f, 1000.0f);
        
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    return true;
}

void main_loop()
{
    input::capture_events();

    long ticks = SDL_GetTicks();

    TIME_DELTA = (ticks - PREV_SDL_TICKS) / 1000.0;
    TIME = ticks / 1000.0;

    PREV_SDL_TICKS = ticks;

    glClear(GL_DEPTH_BUFFER_BIT);

    try {
        SCRIPT_ROOT->eval();

    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    // Update the screen
    SDL_GL_SwapBuffers();

    SDL_Delay(5);
}

int main( int argc, char* args[] )
{
    // Initialize stuff
    circa::initialize();

    SCRIPT_ROOT = &create_branch(circa::KERNEL, "c2d_root");

    input::initialize(*SCRIPT_ROOT);

    gl_shapes::register_functions(*SCRIPT_ROOT);
    mesh::register_functions(*SCRIPT_ROOT);
    textures::register_functions(*SCRIPT_ROOT);

    // Import constants
    expose_value(SCRIPT_ROOT, &TIME, "time");
    expose_value(SCRIPT_ROOT, &TIME_DELTA, "time_delta");

    // Load runtime.ca
    std::string circa_home = getenv("CIRCA_HOME");
    parse_script(*SCRIPT_ROOT, circa_home + "/cuttlefish/runtime.ca");

    // Load user's script
    USERS_BRANCH = &SCRIPT_ROOT->get("users_branch")->asBranch();

    if (argc > 1) {
        std::string filename = args[1];
        std::cout << "Loading file: " << filename << std::endl;
        parse_script(*USERS_BRANCH, filename);
    }

    // Try to initialize display
    if (!initialize_display())
        return 1;

    PREV_SDL_TICKS = SDL_GetTicks();

    // Main loop
    while (CONTINUE_MAIN_LOOP)
        main_loop();

    // Quit SDL
    SDL_Quit();

    return 0;
}
