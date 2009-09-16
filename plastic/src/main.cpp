// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "common_headers.h"

#include <SDL.h>
#include <SDL_opengl.h>

#include <circa.h>

#include "gl_shapes.h"
#include "input.h"
#include "main.h"
#include "mesh.h"
#include "shaders.h"
#include "textures.h"
#include "ttf.h"


using namespace circa;

const int SCREEN_BPP = 32;

SDL_Surface* SCREEN = NULL;
Branch* SCRIPT_ROOT = NULL;
Branch* USERS_BRANCH = NULL;
bool CONTINUE_MAIN_LOOP = true;

Float TIME;
Float TIME_DELTA;
long PREV_SDL_TICKS = 0;

bool PAUSED = false;
PauseReason PAUSE_REASON;

bool initialize_plastic()
{
    // Initialize Circa
    circa::initialize();

    SCRIPT_ROOT = &create_branch(*circa::KERNEL, "plastic_main");

    input::initialize(*SCRIPT_ROOT);
    ttf::initialize(*SCRIPT_ROOT);

    // Import constants
    TIME = float_value(*SCRIPT_ROOT, 0, "time");
    TIME_DELTA = float_value(*SCRIPT_ROOT, 0, "time_delta");

    // Load runtime.ca
    std::string circa_home = getenv("CIRCA_HOME");
    parse_script(*SCRIPT_ROOT, circa_home + "/plastic/runtime.ca");

    input::setup(*SCRIPT_ROOT);
    mesh::setup(*SCRIPT_ROOT);
    gl_shapes::setup(*SCRIPT_ROOT);
    textures::setup(*SCRIPT_ROOT);
    ttf::setup(*SCRIPT_ROOT);

    if (has_static_errors(*SCRIPT_ROOT)) {
        std::cout << "Errors in runtime.ca:" << std::endl;
        print_static_errors_formatted(*SCRIPT_ROOT, std::cout);
        return false;
    }

    return true;
}

bool initialize_display()
{
    // Initialize the window
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Create the surface
    int windowWidth = 640;
    int windowHeight = 480;

    if (USERS_BRANCH->contains("desired_window_size")) {
        windowWidth = (*USERS_BRANCH)["desired_window_size"]->asBranch()[0]->asInt();
        windowHeight = (*USERS_BRANCH)["desired_window_size"]->asBranch()[1]->asInt();
    }

    SCREEN = SDL_SetVideoMode(windowWidth, windowHeight, 16, SDL_OPENGL | SDL_SWSURFACE);

    if (SCREEN == NULL) {
        std::cerr << "SDL_SetVideoMode failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Write window width & height to runtime.ca
    (*SCRIPT_ROOT)["window"]->asBranch()["width"]->asInt() = windowWidth;
    (*SCRIPT_ROOT)["window"]->asBranch()["height"]->asInt() = windowHeight;

    // Initialize desired SDL subsystems
    if (SDL_Init(SDL_INIT_TIMER & SDL_INIT_VIDEO & SDL_INIT_JOYSTICK & SDL_INIT_EVENTTHREAD) == -1)
        return false;

    // Set window caption
    std::string windowTitle;
    if (USERS_BRANCH->contains("desired_window_title"))
        windowTitle = (*USERS_BRANCH)["desired_window_title"]->asString();
    else
        windowTitle = get_branch_source_filename(*USERS_BRANCH);

    SDL_WM_SetCaption(windowTitle.c_str(), NULL);

    // Initialize GL state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT, GL_FILL);
    glDisable(GL_CULL_FACE);
    glClearColor(0,0,0,0);
    glClearDepth(1000);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_TEXTURE);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     
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

    if (!PAUSED) {

        TIME_DELTA = (ticks - PREV_SDL_TICKS) / 1000.0f;
        TIME = ticks / 1000.0f;

        PREV_SDL_TICKS = ticks;

        glClear(GL_DEPTH_BUFFER_BIT);

        static Ref errorListener = new Term();
        errorListener->hasError = false;

        evaluate_branch(*SCRIPT_ROOT, errorListener);

        if (errorListener->hasError) {
            std::cout << "Runtime error:" << std::endl;
            std::cout << errorListener->getErrorMessage() << std::endl;
            PAUSED = true;
            PAUSE_REASON = RUNTIME_ERROR;
        }
    }

    // Update the screen
    SDL_GL_SwapBuffers();

    long new_ticks = SDL_GetTicks();

    // Delay to limit framerate to 60 FPS
    const float target_fps = 60.0;
    const long ticks_per_second = long(1.0 / target_fps * 1000);
    if ((new_ticks - ticks) < ticks_per_second) {
        long delay = ticks_per_second - (new_ticks - ticks);
        SDL_Delay(delay);
    }
}

int plastic_main(std::vector<std::string> args)
{
    if (!initialize_plastic()) return 1;

    // For no args, default action is to run tests
    if (args.size() == 0)
        return circa::run_command_line(args);

    // Inject the requested filename, so that the user's script will be loaded
    String user_script_filename(*SCRIPT_ROOT, "user_script_filename", "");
    user_script_filename = args[0];
    Term* users_branch = SCRIPT_ROOT->get("users_branch");
    include_function::possibly_expand(users_branch);
    USERS_BRANCH = &users_branch->asBranch();

    if (has_static_errors(*USERS_BRANCH)) {
        print_static_errors_formatted(*USERS_BRANCH, std::cout);
        return 1;
    }
    
    // Try to initialize display
    if (!initialize_display()) return 1;

    PREV_SDL_TICKS = SDL_GetTicks();

    // Main loop
    while (CONTINUE_MAIN_LOOP)
        main_loop();

    // Quit SDL
    SDL_Quit();

    return 0;
}

int main( int argc, char* args[] )
{
    std::vector<std::string> argv;

    for (int i = 1; i < argc; i++)
        argv.push_back(args[i]);

    return plastic_main(argv);
}
