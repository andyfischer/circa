// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

void escape_string_for_json(std::string s, std::stringstream& out)
{
    for (size_t i=0; i < s.length(); i++)
    {
        char c = s[i];

        if (c == '\n')
            out << "\\n";
        else if (c == '"')
            out << "\"";
        else
            out << c;
    }
}

std::string escape_newlines(std::string s)
{
    std::stringstream out;

    for (unsigned i=0; i < s.length(); i++) {
        if (s[i] == '\n')
            out << "\\n";
        else
            out << s[i];
    }
    return out.str();
}

bool hidden_from_documentation(Term* term)
{
    if (is_hidden(term))
        return true;

    if (term->boolPropOptional("docs:hidden", false))
        return true;

    return false;
}

void generate_docs_for_function(Term* func, std::stringstream &out)
{
    std::string header = function_t::get_header_source(func);
    std::string description = function_t::get_documentation(func);
    if (description == "") description = "No description provided";

    out << "{\"name\":\"" << func->name << "\"";
    out << ", \"function\":true";
    out << ", \"return_type\": \"" << function_t::get_output_type(func)->name << "\"";
    out << ", \"declaration\": \"" << header << "\"";
    //escape_string_for_json(get_term_source(func), out);
    out << ", \"comments\": \"";
    escape_string_for_json(description, out);
    out << "\"";
    out << "}";
}

void append_package_docs(std::stringstream& out, Branch& branch, std::string const& package_name)
{
    // Language builtins
    out << "    {\"name\": \""<<package_name<<"\", \"contents\":\n";
    out << "[\n";
    bool needs_comma = false;
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (hidden_from_documentation(term)) continue;
        if (is_function(term)) {
            if (needs_comma) out << ",\n";
            generate_docs_for_function(term, out);
            needs_comma = true;
        }
    }
    out << "\n]\n";
    out << "}\n";
}

void hide_from_docs(Term* term)
{
    if (term == NULL) return;
    term->setBoolProp("docs:hidden", true);
}

void initialize_kernel_documentation(Branch& KERNEL)
{
    hide_from_docs(KERNEL["annotate_type"]);
    hide_from_docs(KERNEL["add_feedback"]);
    hide_from_docs(KERNEL["assign"]);
    hide_from_docs(KERNEL["branch"]);
    hide_from_docs(KERNEL["sin_feedback"]);
    hide_from_docs(KERNEL["comment"]);
    hide_from_docs(KERNEL["copy"]);
    hide_from_docs(KERNEL["cos_feedback"]);
    hide_from_docs(KERNEL["do_once"]);
    hide_from_docs(KERNEL["feedback"]);
    hide_from_docs(KERNEL["for"]);
    hide_from_docs(KERNEL["get_field_by_name"]);
    hide_from_docs(KERNEL["get_index"]);
    hide_from_docs(KERNEL["if_feedback"]);
    hide_from_docs(KERNEL["mult_feedback"]);
    hide_from_docs(KERNEL["eval_script"]);
    hide_from_docs(KERNEL["set_field"]);
    hide_from_docs(KERNEL["set_index"]);
    hide_from_docs(KERNEL["stateful_value"]);
    hide_from_docs(KERNEL["if_block"]);
    hide_from_docs(KERNEL["if"]);
    hide_from_docs(KERNEL["input_placeholder"]);
    hide_from_docs(KERNEL["cond_feedback"]);
    hide_from_docs(KERNEL["term_to_source"]);
    hide_from_docs(KERNEL["ref"]);
    hide_from_docs(KERNEL["one_time_assign"]);
    hide_from_docs(KERNEL["unique_id"]);
    hide_from_docs(KERNEL["unknown_field"]);
    hide_from_docs(KERNEL["unknown_function"]);
    hide_from_docs(KERNEL["unknown_identifier"]);
    hide_from_docs(KERNEL["unrecognized_expr"]);
    hide_from_docs(KERNEL["unknown_type"]);
    hide_from_docs(KERNEL["vectorize_vs"]);
    hide_from_docs(KERNEL["vectorize_vv"]);
}

} // namespace circa
