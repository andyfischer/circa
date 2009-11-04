// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

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

#endif
