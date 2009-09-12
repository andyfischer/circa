// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {

void generate_docs_for_function(Term* func, std::stringstream &out)
{
    std::string description = function_t::get_description(func);
    if (description == "") description = "No description provided";

    out << "{'name':'" << func->name << "'";
    out << ", 'function':true'";
    out << ", 'return_type': '" << function_t::get_output_type(func)->name << "'";
    out << ", 'declaration': '" << get_term_source(func) << "'";
    out << ", 'description': '" << description << "'";
    out << "}";
}

void generate_docs(Branch& branch, std::stringstream &out)
{
    out << "[";

    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term == NULL) continue;
        if (is_function(term)) {
            generate_docs_for_function(term, out);
            out << ",\n";
        }
    }

    out << "]\n";
}

} // namespace circa
