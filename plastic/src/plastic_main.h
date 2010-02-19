// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef PLASTIC_MAIN_INCLUDED
#define PLASTIC_MAIN_INCLUDED

#include "plastic_common_headers.h"

extern bool CONTINUE_MAIN_LOOP;
extern int TARGET_FPS;

extern bool PAUSED;
enum PauseReason { USER_REQUEST, RUNTIME_ERROR };
extern PauseReason PAUSE_REASON;

circa::Branch& runtime_branch();
circa::Branch& users_branch();

std::string find_runtime_file();
std::string find_asset_file(std::string const& filename);
bool evaluate_main_script();

// the filename of this binary, passed in as args[0]
extern std::string BINARY_NAME;

bool reload_runtime();
int plastic_main(std::vector<std::string> args);

#endif
