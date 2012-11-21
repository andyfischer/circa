// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "kernel.h"
#include "evaluation.h"
#include "function.h"
#include "importing.h"
#include "importing_macros.h"
#include "parser.h"
#include "names.h"
#include "tagged_value.h"
#include "type.h"
#include "update_cascades.h"

namespace circa {

Term* import_function(Block* block, EvaluateFunc evaluate, std::string const& header)
{
    Term* result = parser::compile(block, parser::function_decl, header);

    as_function(result)->evaluate = evaluate;
    dirty_bytecode(function_contents(as_function(result)));
    return result;
}

void install_function(Term* function, EvaluateFunc evaluate)
{
    ca_assert(is_function(function));
    as_function(function)->evaluate = evaluate;
    dirty_bytecode(function_contents(function));
}

Term* install_function(Block* block, const char* name, EvaluateFunc evaluate)
{
    Term* term = find_name(block, name_from_string(name));
    if (term == NULL) {
        std::string msg = "Name not found in install_function: ";
        msg += name;
        internal_error(msg.c_str());
    }
    as_function(term)->evaluate = evaluate;
    dirty_bytecode(function_contents(as_function(term)));
    return term;
}

Term* import_type(Block* block, Type* type)
{
    Term* term = create_value(block, TYPES.type, as_cstring(&type->name));
    set_type(term_value(term), type);
    return term;
}

void install_function_list(Block* block, const ImportRecord* list)
{
    while (list->functionName != NULL) {
        install_function(block, list->functionName, list->evaluate);
        list++;
    }
}

} // namespace circa
