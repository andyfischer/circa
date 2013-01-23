// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// Various code for debugging purposes. This code shouldn't have an effect on a
// release build.

#pragma once

#include "common_headers.h"

namespace circa {

// Setting this to true will make us abort trap on the next name lookup.
extern bool DEBUG_TRAP_NAME_LOOKUP;

// Setting this to true will make us abort trap on the next call to raise_error()
extern bool DEBUG_TRAP_RAISE_ERROR;

extern int DEBUG_BREAK_ON_TERM;

// Spit out this block's raw contents to std::cout
void dump(Block& block);
void dump(Block* block);
void dump_with_props(Block& block);
void dump(Term* term);
void dump(caValue& value);
void dump(caValue* value);
void dump(Stack* context);
void dump(caStack* stack);

// Signal that an unexpected error has occurred. Depending on debug settings, this
// will either throw an exception or trigger an assert().
void internal_error(const char* message);
void internal_error(std::string const& message);

void ca_debugger_break();

void perf_stats_dump();
void perf_stats_reset();
void perf_stats_to_list(caValue* list);
void perf_stat_inc(int name);

const int c_firstStatIndex = sym_FirstStatIndex + 1;
const int c_numPerfStats = sym_LastStatIndex - 1 - c_firstStatIndex;
extern uint64 PERF_STATS[c_numPerfStats];

#if CIRCA_ENABLE_PERF_STATS

#define INCREMENT_STAT(x) perf_stat_inc(stat_##x);

#else

#define INCREMENT_STAT(x) ;

#endif

// Internal logging
#if CIRCA_ENABLE_LOGGING

void log_start(int channel, const char* name);
void log_arg(const char* key, const char* val);
void log_arg(const char* key, int val);
void log_finish();
void log_msg(int channel, const char* name);

#else

#define log_start(c,n) ;
#define log_arg(k,v) ;
#define log_finish() ;
#define log_msg(c,n) ;

#endif

} // namespace circa
