// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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

const char* DEBUG_BREAK_ON_TERM = "";

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
    std::cout << value.toString() << std::endl;
}
void dump(caValue* value)
{
    std::cout << value->toString() << std::endl;
}

void dump(EvalContext* context)
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

} // namespace circa
