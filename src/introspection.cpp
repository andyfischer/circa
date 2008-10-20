// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "term.h"

namespace circa {

void print_term_extended(Term* term, std::ostream &output)
{
    std::string name = term->name;
    std::string funcName = term->function->name;
    std::string typeName = term->type->name;

    if (name != "")
        output << "'" << name << "' ";

    output << funcName << "() -> " << typeName;

    output << std::endl;
}

void print_branch_extended(Branch& branch, std::ostream &output)
{
    for (int i=0; i < branch.numTerms(); i++) {
        Term* term = branch[i];

        print_term_extended(term, output);
    }
}


} // namespace circa
