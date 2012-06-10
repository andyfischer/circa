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

// Setting this to true will write to stdout for every write to a Ref value.
extern bool DEBUG_TRACE_ALL_REF_WRITES;

extern bool DEBUG_TRACE_ALL_TERM_DESTRUCTORS;

extern int DEBUG_BREAK_ON_TERM;

// Spit out this branch's raw contents to std::cout
void dump(Branch& branch);
void dump(Branch* branch);
void dump_with_props(Branch& branch);
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

// Internal performance statistics
enum PerformanceStat {
    // Building
    STAT_termsCreated,
    STAT_termPropAdded,
    STAT_termPropAccess,

    // Specific reasons for copy()
    STAT_copy_pushedInputNewFrame,
    STAT_copy_pushedInputMultiNewFrame,
    STAT_copy_pushFrameWithInputs,
    STAT_copy_listDuplicate,

    // Specific reasons for cast()
    STAT_cast_listCast,
    STAT_cast_pushFrameWithInputs,
    STAT_cast_finishFrame,

    // Values (general)
    STAT_valueCreates,
    STAT_valueCopies,
    STAT_valueCasts,
    STAT_valueTouch,

    // List values
    STAT_listsCreated,
    STAT_listsGrown,
    STAT_listSoftCopy,
    STAT_listHardCopy,

    // Dict values
    STAT_dictHardCopy,

    // String values
    STAT_stringCopy,
    STAT_stringCreate,

    // Interpreter
    STAT_stepInterpreter,
    STAT_interpreterCastOutputFromFinishedFrame,
    STAT_branchNameLookups,
    STAT_framesCreated,
    STAT_loopFinishIteration,

    // Function calls
    STAT_dynamicCall,
    STAT_setIndex,
    STAT_setField,

    NUM_PERFORMANCE_STATS
};


void perf_stats_dump();
void perf_stats_reset();

#if CIRCA_ENABLE_PERF_STATS

extern uint64 PERF_STATS[NUM_PERFORMANCE_STATS];

#define INCREMENT_STAT(x) PERF_STATS[STAT_##x]++;

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
