// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include <cassert>

#include "build_options.h"
#include "branch.h"
#include "evaluation.h"
#include "introspection.h"

#include "debug.h"

namespace circa {

bool DEBUG_TRAP_NAME_LOOKUP = false;
bool DEBUG_TRAP_RAISE_ERROR = false;
bool DEBUG_TRACE_ALL_REF_WRITES = false;
bool DEBUG_TRACE_ALL_TERM_DESTRUCTORS = false;

int DEBUG_BREAK_ON_TERM = -1;

uint64 PERF_STATS[NUM_PERFORMANCE_STATS];

void dump(Branch& branch)
{
    print_branch(std::cout, &branch);
}
void dump(Branch* branch)
{
    print_branch(std::cout, branch);
}

void dump_with_props(Branch& branch)
{
    print_branch_with_properties(std::cout, &branch);
}

void dump(Term* term)
{
    print_term(std::cout, term);
}

void dump(caValue& value)
{
    std::cout << to_string(&value) << std::endl;
}
void dump(caValue* value)
{
    std::cout << to_string(value) << std::endl;
}

void dump(Stack* context)
{
    eval_context_print_multiline(std::cout, context);
}

void internal_error(const char* message)
{
    #if CIRCA_ASSERT_ON_ERROR
        std::cerr << "internal error: " << message << std::endl;
        assert(false);
    #else
        throw std::runtime_error(message);
    #endif
}

void internal_error(std::string const& message)
{
    internal_error(message.c_str());
}

void ca_assert_function(bool expr, const char* exprStr, int line, const char* file)
{
    if (!expr) {
        std::stringstream msg;
        msg << "ca_assert(" << exprStr << ") failed in " << file << " line " << line;
        internal_error(msg.str().c_str());
    }
}

void ca_debugger_break()
{
#if CIRCA_TEST_BUILD
    //asm { int 3 };
    internal_error("debugger break");
#else
    internal_error("debugger break");
#endif
}

static const char* perf_stat_get_name(PerformanceStat stat)
{
    switch (stat) {
        case STAT_termsCreated: return "termsCreated";
        case STAT_termPropAdded: return "termPropAdded";
        case STAT_termPropAccess: return "termPropAccess";
        case STAT_copy_pushedInputNewFrame: return "copy_pushedInputNewFrame";
        case STAT_copy_pushedInputMultiNewFrame: return "copy_pushedInputMultiNewFrame";
        case STAT_copy_pushFrameWithInputs: return "copy_pushFrameWithInputs";
        case STAT_copy_listDuplicate: return "copy_listDuplicate";
        case STAT_cast_listCast: return "cast_listCast";
        case STAT_cast_pushFrameWithInputs: return "cast_pushFrameWithInputs";
        case STAT_cast_finishFrame: return "cast_finishFrame";
        case STAT_valueCreates: return "valueCreates";
        case STAT_valueCopies: return "valueCopies";
        case STAT_valueCasts: return "valueCasts";
        case STAT_valueTouch: return "valueTouch";
        case STAT_listsCreated: return "listsCreated";
        case STAT_listsGrown: return "listsGrown";
        case STAT_listSoftCopy: return "listSoftCopy";
        case STAT_listHardCopy: return "listHardCopy";
        case STAT_dictHardCopy: return "dictHardCopy";
        case STAT_stringCopy: return "stringCopy";
        case STAT_stringCreate: return "stringCreate";
        case STAT_stepInterpreter: return "stepInterpreter";
        case STAT_interpreterCastOutputFromFinishedFrame: return "interpreterCastOutputFromFinishedFrame";
        case STAT_branchNameLookups: return "branchNameLookups";
        case STAT_framesCreated: return "framesCreated";
        case STAT_loopFinishIteration: return "loopFinishIteration";
        case STAT_dynamicCall: return "dynamicCall";
        case STAT_setIndex: return "setIndex";
        case STAT_setField: return "setField";
        case NUM_PERFORMANCE_STATS: return "";
    }
    return "(stat not found)";
}

void perf_stats_dump()
{
#if CIRCA_ENABLE_PERF_STATS
    printf("perf_stats_dump:\n");
    for (int i=0; i < NUM_PERFORMANCE_STATS; i++) {
        printf("  %s = %llu\n", perf_stat_get_name((PerformanceStat) i), PERF_STATS[i]);
    }
#endif
}
void perf_stats_reset()
{
#if CIRCA_ENABLE_PERF_STATS
    memset(&PERF_STATS, 0, sizeof(PERF_STATS));
#endif
}

#if CIRCA_ENABLE_LOGGING

FILE* g_logFile = NULL;
int g_logArgCount = 0;
int g_logInProgress = false;

void log_start(int channel, const char* name)
{
    ca_assert(!g_logInProgress);

    if (g_logFile == NULL)
        g_logFile = fopen("circa.log", "w");

    // Ignore channel for now

    // Timestamp
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    char timestamp[100];
    strftime(timestamp, 100, "%H:%M:%S", timeinfo);
    fprintf(g_logFile, "%s %s(", timestamp, name);

    g_logArgCount = 0;
    g_logInProgress = true;
}

void log_arg(const char* key, const char* val)
{
    ca_assert(g_logInProgress);
    if (g_logArgCount > 0)
        fprintf(g_logFile, ", ");

    fprintf(g_logFile, "%s = %s", key, val);
    g_logArgCount++;
}
void log_arg(const char* key, int val)
{
    char buf[32];
    sprintf(buf, "%d", val);
    log_arg(key, buf);
}

void log_finish()
{
    ca_assert(g_logInProgress);
    fprintf(g_logFile, ")\n");
    g_logInProgress = false;
    g_logArgCount = 0;
}
void log_msg(int channel, const char* name)
{
    log_start(channel, name);
    log_finish();
}

#endif

} // namespace circa
