// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include <cassert>

#include "build_options.h"
#include "branch.h"
#include "evaluation.h"
#include "introspection.h"
#include "names_builtin.h"

#include "debug.h"

namespace circa {

bool DEBUG_TRAP_NAME_LOOKUP = false;
bool DEBUG_TRAP_RAISE_ERROR = false;
bool DEBUG_TRACE_ALL_REF_WRITES = false;
bool DEBUG_TRACE_ALL_TERM_DESTRUCTORS = false;

int DEBUG_BREAK_ON_TERM = -1;

uint64 PERF_STATS[c_numPerfStats];

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


void perf_stats_dump()
{
#if CIRCA_ENABLE_PERF_STATS
    printf("perf_stats_dump:\n");
    for (int i=c_firstStatIndex; i < name_LastStatIndex; i++)
        printf("  %s = %llu\n", name_to_string(i), PERF_STATS[i - c_firstStatIndex]);
#endif
}
void perf_stats_reset()
{
#if CIRCA_ENABLE_PERF_STATS
    for (int i = c_firstStatIndex; i < name_LastStatIndex; i++)
        PERF_STATS[i - c_firstStatIndex] = 0;
#endif
}

void perf_stat_inc(int name)
{
    PERF_STATS[name - c_firstStatIndex]++;
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
