// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {

bool hidden_from_documentation(Term* term)
{
    return (term->name == "annotate_type")
        || (term->name == "add_feedback")
        || (term->name == "branch")
        || (term->name == "sin_feedback")
        || (term->name == "comment")
        || (term->name == "cos_feedback")
        || (term->name == "for")
        || (term->name == "if_feedback")
        || (term->name == "mult_feedback")
        || (term->name == "eval_script")
        || (term->name == "stateful_value")
        || (term->name == "if_block")
        || (term->name == "if")
        || (term->name == "input_placeholder")
        || (term->name == "if_expr_feedback")
        || (term->name == "term_to_source")
        || (term->name == "unknown_field")
        || (term->name == "unknown_function")
        || (term->name == "unknown_identifier")
        || (term->name == "unrecognized_expr")
        || (term->name == "unknown_type");
}

void generate_docs_for_function(Term* func, std::stringstream &out)
{
    std::string description = function_t::get_description(func);
    if (description == "") description = "No description provided";

    out << "{\"name\":\"" << func->name << "\"";
    out << ", \"function\":true";
    out << ", \"return_type\": \"" << function_t::get_output_type(func)->name << "\"";
    out << ", \"declaration\": \"" << get_term_source(func) << "\"";
    out << ", \"comments\": \"" << description << "\"";
    out << "}";
}

void generate_docs(Branch& branch, std::stringstream &out)
{
    // Header information
    out << "{\n";
    out << "  \"headers\": { \"title\": \"Circa\" },\n";
    out << "  \"packages\": [\n";
    out << "    {\"name\": \"builtins\", \"contents\":\n";
    out << "[\n";

    bool needs_comma = false;
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term == NULL) continue;
        if (is_hidden(term)) continue;
        if (hidden_from_documentation(term)) continue;
        if (is_function(term)) {
            if (needs_comma) out << ",\n";
            generate_docs_for_function(term, out);
            needs_comma = true;
        }
    }

    out << "\n]\n";
    out << "}\n";
    out << "]\n";
    out << "}\n";
}


} // namespace circa
