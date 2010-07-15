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

    // The FPS that we are trying to maintain
    int _targetFps;

    long _ticksElapsed;

    int _windowWidth;
    int _windowHeight;

    App()
      : _runtimeBranch(NULL),
        _usersBranch(NULL),
        _continueMainLoop(true),
        _targetFps(60),
        _ticksElapsed(0),
        _windowWidth(0),
        _windowHeight(0)
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
void update_window_dimensions(int width, int height);

// Logging
void info(std::string const& msg);
void error(std::string const& msg);

std::string find_runtime_file();
std::string find_asset_file(std::string const& filename);
bool load_user_script_filename(std::string const& _filename);
bool setup_builtin_functions();
bool reload_runtime();

bool initialize();
bool evaluate_main_script();
//bool reload_runtime();

} // namespace app
