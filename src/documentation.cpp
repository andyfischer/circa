// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "kernel.h"
#include "generic.h"
#include "function.h"
#include "source_repro.h"
#include "symbols.h"
#include "term.h"
#include "type.h"

#include "types/ref.h"

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

std::string get_header_source(Term* function)
{
    StyledSource styled;
    function_format_header_source(&styled, as_function(function));
    return unformat_rich_source(&styled);
}

void generate_docs_for_function(Term* func, std::stringstream &out)
{
    std::string header = get_header_source(func);
    std::string description = function_get_documentation_string(as_function(func));
    if (description == "") description = "No description provided";

    out << "{\"name\":\"" << func->name << "\"";
    out << ", \"function\":true";
    out << ", \"return_type\": \"" << symbol_get_text(function_get_output_type(func, 0)->name) << "\"";
    out << ", \"declaration\": \"" << header << "\"";
    //escape_string_for_json(get_term_source(func), out);

    if (is_overloaded_function(as_function(func))) {
        out << ", \"containsOverloads\": [";

        List& overloads = as_function(func)->parameters;
        for (int overload=0; overload < overloads.length(); overload++) {
            if (overload != 0)
                out << ", ";
            Term* overloadTerm = as_ref(overloads[overload]);
            out << '"' << overloadTerm->name << '"';

            ca_assert(overloadTerm->name != "");
        }
        out << "]";
    }

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

} // namespace circa
