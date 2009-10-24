// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include <circa.h>

#include "plastic.h"

using namespace circa;

// the filename of this binary, passed in as args[0]
std::string BINARY_NAME;

const int SCREEN_BPP = 32;

SDL_Surface* SCREEN = NULL;
Branch* SCRIPT_ROOT = NULL;
Branch* USERS_BRANCH = NULL;
bool CONTINUE_MAIN_LOOP = true;

Float TIME;
Float TIME_DELTA;
long PREV_SDL_TICKS = 0;
int TARGET_FPS = 60;

int WINDOW_WIDTH = 0;
int WINDOW_HEIGHT = 0;

bool PAUSED = false;
PauseReason PAUSE_REASON;

std::string get_home_directory()
{
    char* circa_home = getenv("CIRCA_HOME");
    if (circa_home == NULL) {
        return get_directory_for_filename(BINARY_NAME);
    } else {
        return std::string(circa_home) + "/plastic";
    }
}

std::string find_runtime_file()
{
    return get_home_directory() + "/runtime.ca";
}

std::string find_asset_file(std::string const& filename)
{
    return get_home_directory() + "/" + filename;
}

bool initialize_plastic()
{
    // Initialize Circa
    circa::initialize();

    SCRIPT_ROOT = &create_branch(*circa::KERNEL, "plastic_main");

    input::initialize(*SCRIPT_ROOT);

    // Import constants
    TIME = float_value(*SCRIPT_ROOT, 0, "time");
    TIME_DELTA = float_value(*SCRIPT_ROOT, 0, "time_delta");

    // Load runtime.ca
    std::string runtime_ca_path = find_runtime_file();
    if (!file_exists(runtime_ca_path)) {
        std::cout << "fatal: Couldn't find runtime.ca file" << std::endl;
        return false;
    }
    parse_script(*SCRIPT_ROOT, runtime_ca_path);

    postprocess_functions::setup(*SCRIPT_ROOT);
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
    WINDOW_WIDTH = 640;
    WINDOW_HEIGHT = 480;

    if (USERS_BRANCH->contains("desired_window_size")) {
        WINDOW_WIDTH = (*USERS_BRANCH)["desired_window_size"]->asBranch()[0]->asInt();
        WINDOW_HEIGHT = (*USERS_BRANCH)["desired_window_size"]->asBranch()[1]->asInt();
    }

    SCREEN = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 16, SDL_OPENGL | SDL_SWSURFACE);

    if (SCREEN == NULL) {
        std::cerr << "SDL_SetVideoMode failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Write window width & height to runtime.ca
    (*SCRIPT_ROOT)["window"]->asBranch()["width"]->asInt() = WINDOW_WIDTH;
    (*SCRIPT_ROOT)["window"]->asBranch()["height"]->asInt() = WINDOW_HEIGHT;

    // Initialize desired SDL subsystems
    if (SDL_Init(SDL_INIT_TIMER & SDL_INIT_VIDEO
                & SDL_INIT_JOYSTICK & SDL_INIT_EVENTTHREAD) == -1) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set window caption
    std::string windowTitle;
    if (USERS_BRANCH->contains("desired_window_title"))
        windowTitle = (*USERS_BRANCH)["desired_window_title"]->asString();
    else
        windowTitle = get_branch_source_filename(*USERS_BRANCH);

    SDL_WM_SetCaption(windowTitle.c_str(), NULL);

    // Initialize GL state
    gl_clear_error();
    Term errorListener;

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

    gl_check_error(&errorListener, " (initialize display)");

    if (errorListener.hasError()) {
        std::cerr << "GL error during initialization: "
            << get_runtime_error_message(&errorListener) << std::endl;
        return false;
    }

    return true;
}

void main_loop()
{
    input::capture_events();

    long ticks = SDL_GetTicks();

    gl_clear_error();

    // Evaluate script
    if (!PAUSED) {

        TIME_DELTA = (ticks - PREV_SDL_TICKS) / 1000.0f;
        TIME = ticks / 1000.0f;

        PREV_SDL_TICKS = ticks;

        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(0);

        Term errorListener;

        evaluate_branch(*SCRIPT_ROOT, &errorListener);

        if (errorListener.hasError()) {
            std::cout << "Runtime error:" << std::endl;
            print_runtime_error_formatted(*SCRIPT_ROOT, std::cout);
            std::cout << std::endl;
            PAUSED = true;
            PAUSE_REASON = RUNTIME_ERROR;
        }
    }

    // Check for uncaught GL error
    Term errorListener;
    gl_check_error(&errorListener, " (uncaught)");
    if (errorListener.hasError()) {
        std::cout << get_runtime_error_message(&errorListener) << std::endl;
    }

    // Update the screen
    SDL_GL_SwapBuffers();

    long new_ticks = SDL_GetTicks();

    // Delay to limit framerate
    const long ticks_per_second = long(1.0 / TARGET_FPS * 1000);
    if ((new_ticks - ticks) < ticks_per_second) {
        long delay = ticks_per_second - (new_ticks - ticks);
        SDL_Delay(delay);
    }
}

bool load_user_script_filename(std::string const& filename)
{
    SCRIPT_ROOT->get("user_script_filename")->asString() = filename;
    Term* users_branch = SCRIPT_ROOT->get("users_branch");
    include_function::load_script(users_branch);
    USERS_BRANCH = &users_branch->asBranch();

    return true;
}

int plastic_main(std::vector<std::string> args)
{
    // For no args, default action is to run tests
    if (args.size() == 0)
        return circa::run_command_line(args);

    if (!initialize_plastic()) return 1;

    // Check to generate docs
    if (args[0] == "-gd") {
        std::cout << "writing docs to " << args[1] << std::endl;
        std::stringstream out;
        generate_plastic_docs(out);
        write_text_file(args[1], out.str());
        return 0;
    }

    // Print compiled code
    if (args[0] == "-p") {
        if (!load_user_script_filename(args[1]))
            return 1;
        std::cout << print_branch_raw(*SCRIPT_ROOT);
        return 0;
    }

    if (!load_user_script_filename(args[0]))
        return 1;

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
    BINARY_NAME = args[0];

    std::vector<std::string> argv;

    for (int i = 1; i < argc; i++)
        argv.push_back(args[i]);

    return plastic_main(argv);
}

