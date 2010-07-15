// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>
#include <importing_macros.h>

#include "app.h"
#include "gl_shapes.h"
#include "ide.h"
#include "image.h"
#include "input.h"
#include "mesh.h"
#include "postprocess.h"
#include "text.h"
#include "textures.h"

namespace app {

App& singleton()
{
    static App* obj = new App();
    return *obj;
}

circa::Branch& runtime_branch()
{
    return *singleton()._runtimeBranch;
}

circa::Branch& users_branch()
{
    return *singleton()._usersBranch;
}

bool paused()
{
    return singleton()._pauseStatus.paused;
}

PauseStatus::Reason pause_reason()
{
    return singleton()._pauseStatus.reason;
}

bool continue_main_loop()
{
    return singleton()._continueMainLoop;
}

void pause(PauseStatus::Reason reason)
{
    singleton()._pauseStatus.paused = true;
    singleton()._pauseStatus.reason = reason;
}

void unpause()
{
    singleton()._pauseStatus.paused = false;
    singleton()._pauseStatus.reason = PauseStatus::NONE;
}

void update_window_dimensions(int width, int height)
{
    singleton()._windowWidth = width;
    singleton()._windowHeight = height;
}

void info(std::string const& msg)
{
#ifdef PLATFORM_IPAD
    NSLog(msg.c_str());
#else
    std::cout << msg << std::endl;
#endif
}

void error(std::string const& msg)
{
#ifdef PLATFORM_IPAD
    std::string s = "error: " + msg;
    NSLog(s.c_str());
#else
    std::cout << "error: " << msg << std::endl;
#endif
}

CA_FUNCTION(trace)
{
    std::stringstream out;
    for (int i = 0; i < NUM_INPUTS; i++) {
        if (INPUT(i)->value_type == circa::type_contents(circa::STRING_TYPE))
            out << circa::as_string(INPUT(i));
        else
            out << INPUT(i)->toString();
    }
    info(out.str());
}

std::string get_home_directory()
{
    char* circa_home = getenv("CIRCA_HOME");
    if (circa_home == NULL) {
        return circa::get_directory_for_filename(app::singleton()._binaryFilename);
    } else {
        return std::string(circa_home) + "/plastic";
    }
}

std::string find_runtime_file()
{
#ifdef PLASTIC_IPAD
    return get_home_directory() + "/runtime/main_ipad.ca";
#else
    return get_home_directory() + "/runtime/main.ca";
#endif
}

std::string find_asset_file(std::string const& filename)
{
    return get_home_directory() + "/" + filename;
}

bool load_runtime()
{
    // Pre-setup
#ifndef PLASTIC_IPAD
    text::pre_setup(app::runtime_branch());
#endif

    // Load runtime.ca
    std::string runtime_ca_path = find_runtime_file();
    if (!circa::file_exists(runtime_ca_path)) {
        std::cerr << "fatal: Couldn't find runtime.ca file. (expected at "
            << runtime_ca_path << ")" << std::endl;
        return false;
    }
    parse_script(app::runtime_branch(), runtime_ca_path);

    assert(branch_check_invariants(app::runtime_branch(), &std::cout));

    return true;
}

bool initialize()
{
    circa::initialize();

    // Patch the trace() function to use our logging
    install_function(circa::get_global("print"), trace);
    install_function(circa::get_global("trace"), trace);

    app::singleton()._runtimeBranch = &create_branch(*circa::KERNEL, "plastic_main");
    return load_runtime();
}

bool setup_builtin_functions()
{
    circa::Branch& branch = app::runtime_branch();

    ide::setup(branch);
    gl_shapes::setup(branch);

#ifdef PLASTIC_USE_SDL
    postprocess_functions::setup(branch);
    image::setup(branch);
    input::setup(branch);
    mesh::setup(branch);
    textures::setup(branch);
    text::setup(branch);
#endif

    if (circa::has_static_errors(branch)) {
        circa::print_static_errors_formatted(branch, std::cout);
        std::cout << std::endl;
        return false;
    }

    return true;
}

bool reload_runtime()
{
    app::runtime_branch().clear();
    if (!load_runtime())
        return false;
    if (!setup_builtin_functions())
        return false;

    // Write window width & height
    set_int(app::runtime_branch()["window"]->getField("width"), app::singleton()._windowWidth);
    set_int(app::runtime_branch()["window"]->getField("height"), app::singleton()._windowHeight);

    return true;
}

bool load_user_script_filename(std::string const& _filename)
{
    circa::Term* users_branch = app::runtime_branch()["users_branch"];
    app::singleton()._usersBranch = &users_branch->asBranch();

    if (_filename != "") {
        std::string filename = circa::get_absolute_path(_filename);

        circa::Term* user_script_filename = app::runtime_branch().findFirstBinding("user_script_filename");
        circa::set_str(user_script_filename, filename);
        circa::mark_stateful_value_assigned(user_script_filename);

        std::stringstream msg;
        msg << "Loading script: " << filename;
        info(msg.str());
    }

    return true;
}

bool evaluate_main_script()
{
    circa::EvalContext context;
    circa::evaluate_branch(&context, app::runtime_branch());

    if (context.errorOccurred) {
        std::stringstream msg;
        msg << "Runtime error:" << std::endl;
        circa::print_runtime_error_formatted(context, msg);
        info(msg.str().c_str());

        app::pause(PauseStatus::RUNTIME_ERROR);
        return false;
    }

    return true;
}

} // namespace app
