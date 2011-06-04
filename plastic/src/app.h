// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "plastic_common_headers.h"

#include "pause_status.h"

namespace app {

struct App {

    typedef void (*OnFrameCallbackFunc)(void* userdata, App* app, int event);

    struct OnFrameCallback {
        OnFrameCallbackFunc func;
        void* userdata;

        OnFrameCallback(OnFrameCallbackFunc f, void* u) : func(f), userdata(u) {}
    };

    circa::Branch* _runtimeBranch;
    circa::Branch* _usersBranch;

    circa::EvalContext _evalContext;

    // the filename of the running binary, passed in as args[0]
    std::string _binaryFilename;

    // the script filename that we should start with
    std::string _initialScriptFilename;

    PauseStatus _pauseStatus;

    bool _continueMainLoop;

    // The FPS that we are trying to maintain
    int _targetFps;

    long _ticksElapsed;

    int _windowWidth;
    int _windowHeight;

    std::vector<OnFrameCallback> preFrameCallbacks;
    std::vector<OnFrameCallback> postFrameCallbacks;

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

    void setScriptFilename(const std::string& filename);

    void addPreFrameCallback(OnFrameCallbackFunc func, void* userdata);
    void addPostFrameCallback(OnFrameCallbackFunc func, void* userdata);
};

App& get_global_app();
circa::Branch& runtime_branch();
circa::Branch& users_script();
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
bool setup_functions(circa::Branch& runtime);
bool reload_runtime();

bool initialize();
void shutdown();
bool evaluate_main_script();
void update_time_from_elapsed_millis(int elapsed_millis);

} // namespace app
