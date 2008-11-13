// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "branch.h"
#include "introspection.h"
#include "term.h"

namespace circa {

void print_source_line(Term* term, std::ostream &output)
{
    if (term->name != "")
        output << term->name << " = ";

    output << get_short_local_name(term->function);

    output << "(";

    bool first_input = true;
    for (unsigned int input_index=0; input_index < term->inputs.count(); input_index++) {
        Term* input = term->inputs[input_index];
        if (!first_input) output << ", ";
        output << get_short_local_name(input);
        first_input = false;
    }

    output << ")" << std::endl;
}

void print_source(Branch& branch, std::ostream &output)
{
    for (int i=0; i < branch.numTerms(); i++) {
        print_source_line(branch[i], output);
    }
}

} // namespace circa
