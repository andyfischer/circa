// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

#include "plastic.h"

using namespace circa;

// the filename of this binary, passed in as args[0]
std::string BINARY_NAME;

Branch* RUNTIME_BRANCH = NULL;
Branch* USERS_BRANCH = NULL;

bool CONTINUE_MAIN_LOOP = true;

Float TIME;
Float TIME_DELTA;
long PREV_SDL_TICKS = 0;
int TARGET_FPS = 60;

bool PAUSED = false;
PauseReason PAUSE_REASON;

circa::Branch& runtime_branch()
{
    return *RUNTIME_BRANCH;
}

circa::Branch& users_branch()
{
    return *USERS_BRANCH;
}

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

    RUNTIME_BRANCH = &create_branch(*circa::KERNEL, "plastic_main");

    input::initialize(runtime_branch());

    // Import constants
    TIME = float_value(runtime_branch(), 0, "time");
    TIME_DELTA = float_value(runtime_branch(), 0, "time_delta");

    // Load runtime.ca
    std::string runtime_ca_path = find_runtime_file();
    if (!file_exists(runtime_ca_path)) {
        std::cout << "fatal: Couldn't find runtime.ca file" << std::endl;
        return false;
    }
    parse_script(runtime_branch(), runtime_ca_path);

    postprocess_functions::setup(runtime_branch());
    input::setup(runtime_branch());
    mesh::setup(runtime_branch());
    gl_shapes::setup(runtime_branch());
    textures::setup(runtime_branch());
    ttf::setup(runtime_branch());

    if (has_static_errors(runtime_branch())) {
        std::cout << "Errors in runtime.ca:" << std::endl;
        print_static_errors_formatted(runtime_branch(), std::cout);
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
    }

    render_frame();

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
    runtime_branch()["user_script_filename"]->asString() = filename;
    Term* users_branch = runtime_branch()["users_branch"];
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
        std::cout << print_branch_raw(runtime_branch());
        return 0;
    }

    if (!load_user_script_filename(args[0]))
        return 1;

    if (has_static_errors(users_branch())) {
        print_static_errors_formatted(users_branch(), std::cout);
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

