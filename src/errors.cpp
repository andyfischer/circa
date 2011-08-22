// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include <cassert>

#include "build_options.h"
#include "builtins.h"
#include "code_iterators.h"
#include "errors.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "source_repro.h"
#include "type.h"

namespace circa {

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
        << " " << context_get_error_message(&context);
}

CA_FUNCTION(evaluate_func_for_static_error)
{
    // No-op for now
}

} // namespace circa
