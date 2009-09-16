// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef PLASTIC_MAIN_INCLUDED
#define PLASTIC_MAIN_INCLUDED

extern bool CONTINUE_MAIN_LOOP;
extern circa::Branch* USERS_BRANCH;
extern circa::Branch* SCRIPT_ROOT;

extern bool PAUSED;
enum PauseReason { USER_REQUEST, RUNTIME_ERROR };
extern PauseReason PAUSE_REASON;

#endif
