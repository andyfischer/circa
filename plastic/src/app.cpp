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

void error(std::string const& msg)
{
    std::cout << "error: " << msg << std::endl;
}

} // namespace app
