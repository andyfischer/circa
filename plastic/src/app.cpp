// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

#include "app.h"

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

#if 0
bool reload_runtime()
{
    app::runtime_branch().clear();
    if (!load_runtime())
        return false;
    if (!setup_builtin_functions())
        return false;

    // Write window width & height
    set_int(app::runtime_branch()["window"]->getField("width"), WINDOW_WIDTH);
    set_int(app::runtime_branch()["window"]->getField("height"), WINDOW_HEIGHT);

    return true;
}
#endif

} // namespace app
