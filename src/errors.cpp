// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <cassert>

#include "branch_iterator.h"
#include "build_options.h"
#include "errors.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "source_repro.h"
#include "type.h"

namespace circa {

void error_occurred(EvalContext* context, Term* errorTerm, std::string const& message)
{
    if (DEBUG_TRAP_ERROR_OCCURRED)
        ca_assert(false);

    ca_assert(errorTerm != NULL);

    if (context == NULL)
        throw std::runtime_error(message);

    if (!context->errorOccurred) {
        context->errorOccurred = true;
        context->errorTerm = errorTerm;
        context->errorMessage = message;
    }
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

void print_runtime_error_formatted(EvalContext& context, std::ostream& output)
{
    output << get_short_location(context.errorTerm)
        << " " << context.errorMessage;
}

} // namespace circa
