// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "plastic_common_headers.h"

#include "pause_status.h"

namespace app {

struct App {

    circa::Branch* _runtimeBranch;
    circa::Branch* _usersBranch;

    // the filename of the running binary, passed in as args[0]
    std::string _binaryFilename;

    PauseStatus _pauseStatus;

    bool _continueMainLoop;

    App()
      : _runtimeBranch(NULL),
        _usersBranch(NULL),
        _continueMainLoop(true)
    {
    }
};

App& singleton();

circa::Branch& runtime_branch();
circa::Branch& users_branch();
bool paused();
PauseStatus::Reason pause_reason();
bool continue_main_loop();

void pause(PauseStatus::Reason reason);
void unpause();

} // namespace app
