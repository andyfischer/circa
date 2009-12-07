// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "plastic.h"

#include "ide.h"
#include "ttf.h"

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
    std::cout << "initialize_plastic()" << std::endl;

    // Initialize Circa
    circa::initialize();

    RUNTIME_BRANCH = &create_branch(*circa::KERNEL, "plastic_main");

    // Load runtime.ca
    std::string runtime_ca_path = find_runtime_file();
    if (!file_exists(runtime_ca_path)) {
        std::cerr << "fatal: Couldn't find runtime.ca file. (expected at "
            << runtime_ca_path << ")" << std::endl;
        return false;
    }
    parse_script(runtime_branch(), runtime_ca_path);

    // Fetch constants
    TIME = procure_value(runtime_branch(), FLOAT_TYPE, "time");
    TIME_DELTA = procure_value(runtime_branch(), FLOAT_TYPE, "time_delta");

    return true;
}

bool initialize_builtin_functions()
{
    postprocess_functions::setup(runtime_branch());
    ide::setup(runtime_branch());
    image::setup(runtime_branch());
    input::setup(runtime_branch());
    mesh::setup(runtime_branch());
    gl_shapes::setup(runtime_branch());
    textures::setup(runtime_branch());
    ttf::setup(runtime_branch());

    if (has_static_errors(runtime_branch())) {
        std::cout << "Errors in runtime.ca:" << std::endl;
        print_errors_formatted(runtime_branch(), std::cout);
        std::cout << std::endl;
        return false;
    }

    return true;
}

bool evaluate_main_script()
{
    Term errorListener;
    evaluate_branch(runtime_branch(), &errorListener);

    if (errorListener.hasError()) {
        std::cout << "Runtime error:" << std::endl;
        print_runtime_error_formatted(runtime_branch(), std::cout);
        std::cout << std::endl;
        PAUSED = true;
        PAUSE_REASON = RUNTIME_ERROR;
        clear_error(&errorListener);
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

bool load_user_script_filename(std::string const& _filename)
{
    Term* users_branch = runtime_branch()["users_branch"];
    USERS_BRANCH = &users_branch->asBranch();

    if (_filename != "") {
        std::string filename = get_absolute_path(_filename);
        runtime_branch()["user_script_filename"]->asString() = filename;
        std::cout << "Loading script: " << filename << std::endl;
        
        include_function::load_script(users_branch);
    }

    return true;
}

int plastic_main(std::vector<std::string> args)
{
    if (!initialize_plastic()) return 1;

    std::string arg0;
    if (args.size() > 0) arg0 = args[0];

    // -gd to generate docs
    if (arg0 == "-gd") {
        std::cout << "writing docs to " << args[1] << std::endl;
        std::stringstream out;
        generate_plastic_docs(out);
        write_text_file(args[1], out.str());
        return 0;
    }

    // -p to print raw compiled code
    if (arg0 == "-p") {
        if (!load_user_script_filename(args[1]))
            return 1;
        std::cout << print_branch_raw(users_branch());
        return 0;
    }

    // -tr to do a test run of a script, without creating a display.
    if (arg0 == "-tr") {
        if (!load_user_script_filename(args[1]))
            return 1;

        if (has_static_errors(users_branch())) {
            print_static_errors_formatted(users_branch(), std::cout);
            return 1;
        }

        for (int i=0; i < 5; i++)
            if (!evaluate_main_script())
                return 1;

        return 0;
    }

    // Normal operation, load the script file in argument 0.
    std::string filename = arg0;
    if (filename == "") {
        Term* default_script_filename = runtime_branch()["default_script_filename"];
        evaluate_term(default_script_filename);
        filename = default_script_filename->asString();
    }

    if (!load_user_script_filename(filename))
        return 1;

    if (has_static_errors(users_branch())) {
        print_static_errors_formatted(users_branch(), std::cout);
        return 1;
    }

    if (!initialize_builtin_functions()) return 1;
    if (!initialize_display()) return 1;

    PREV_SDL_TICKS = SDL_GetTicks();

    // Main loop
    while (CONTINUE_MAIN_LOOP)
        main_loop();

    // Quit SDL
    SDL_Quit();

    return 0;
}
