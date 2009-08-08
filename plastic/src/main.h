// Copyright 2008 Andrew Fischer

#ifndef CUTTLEFISH_MAIN_INCLUDED
#define CUTTLEFISH_MAIN_INCLUDED

extern bool CONTINUE_MAIN_LOOP;
extern circa::Branch* USERS_BRANCH;

extern bool PAUSED;
enum PauseReason { USER_REQUEST, RUNTIME_ERROR };
extern PauseReason PAUSE_REASON;

#endif
