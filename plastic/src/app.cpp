// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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

void
App::setScriptFilename(const std::string& filename)
{
    _initialScriptFilename = filename;
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
        if (INPUT(i)->value_type == circa::unbox_type(circa::STRING_TYPE))
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
    return "main_ipad.ca";
#else
    return get_home_directory() + "/runtime/main.ca";
#endif
}

std::string find_asset_file(std::string const& filename)
{
    return get_home_directory() + "/" + filename;
}

bool load_runtime(circa::Branch& runtime)
{
    // Load runtime.ca
    std::string runtime_ca_path = find_runtime_file();
    info(std::string("loading runtime: ") + runtime_ca_path);

    if (!circa::storage::file_exists(runtime_ca_path.c_str())) {
        std::cerr << "fatal: Couldn't find runtime file. (expected at "
            << runtime_ca_path << ")" << std::endl;
        return false;
    }
    parse_script(runtime, runtime_ca_path);

    ca_assert(branch_check_invariants_print_result(runtime, std::cout));

    setup_functions(app::runtime_branch());

    circa::Term* users_branch = runtime["users_branch"];
    app::singleton()._usersBranch = &users_branch->nestedContents;

    return true;
}

bool initialize()
{
    circa_initialize();

    // Patch the trace() function to use our own logging.
    install_function(circa::get_global("print"), trace);
    install_function(circa::get_global("trace"), trace);

    App& app = app::singleton();

    app._runtimeBranch = &create_branch(*circa::KERNEL, "plastic_main");
    return load_runtime(*app._runtimeBranch);
}

bool setup_functions(circa::Branch& runtime)
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
    if (!load_runtime(app::runtime_branch()))
        return false;
    if (!setup_functions(app::runtime_branch()))
        return false;

    // Write window width & height
    set_int(app::runtime_branch()["window"]->getField("width"), app::singleton()._windowWidth);
    set_int(app::runtime_branch()["window"]->getField("height"), app::singleton()._windowHeight);

    return true;
}

bool evaluate_main_script()
{
    App* app = &app::singleton();
    circa::EvalContext* context = &app->_evalContext;

    circa::clear_error(context);

    circa::evaluate_branch_no_preserve_locals(&app->_evalContext, app::runtime_branch());

    if (app->_evalContext.errorOccurred) {
        std::stringstream msg;
        msg << "Runtime error:" << std::endl;
        circa::print_runtime_error_formatted(app->_evalContext, msg);
        info(msg.str().c_str());

        app::pause(PauseStatus::RUNTIME_ERROR);
        return false;
    }

    return true;
}

void update_time_from_elapsed_millis(int elapsed_millis)
{
    static long previousMillis = elapsed_millis;
    int ticksAdvanced = elapsed_millis - previousMillis;
    previousMillis = elapsed_millis;

    if (!app::paused()) {
        app::singleton()._ticksElapsed += ticksAdvanced;
    }
}

} // namespace app
