// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "kernel.h"
#include "function.h"
#include "importing.h"
#include "interpreter.h"
#include "parser.h"
#include "names.h"
#include "tagged_value.h"
#include "type.h"
#include "update_cascades.h"

namespace circa {

Term* import_function(Block* block, EvaluateFunc evaluate, const char* header)
{
    Term* result = parser::compile(block, parser::function_decl, header);

    function_contents(result)->overrides.evaluate = evaluate;
    dirty_bytecode(function_contents(result));
    return result;
}

void install_function(Term* function, EvaluateFunc evaluate)
{
    ca_assert(is_function(function));
    function_contents(function)->overrides.evaluate = evaluate;
    dirty_bytecode(function_contents(function));
}

Term* install_function(Block* block, const char* name, EvaluateFunc evaluate)
{
    Term* term = find_name(block, name);
    if (term == NULL) {
        std::string msg = "Symbol not found in install_function: ";
        msg += name;
        internal_error(msg.c_str());
    }
    function_contents(term)->overrides.evaluate = evaluate;
    dirty_bytecode(function_contents(term));
    return term;
}

Term* import_type(Block* block, Type* type)
{
    Term* term = create_value(block, TYPES.type, as_cstring(&type->name));
    set_type(term_value(term), type);
    return term;
}

} // namespace circa
