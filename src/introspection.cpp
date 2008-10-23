// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "runtime.h"
#include "term.h"
#include "type.h"

namespace circa {

void print_term_extended(Term* term, std::ostream &output)
{
    if (term == NULL) {
        output << "<NULL>";
        return;
    }

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

void print_terms(ReferenceList const& list, std::ostream &output)
{
    for (unsigned int i=0; i < list.count(); i++) {
        print_term_extended(list[i], output);
    }
}

ReferenceList list_all_pointers(Term* term)
{
    ReferenceList result;

    struct AppendPointersToList : PointerVisitor
    {
        ReferenceList& list;
        AppendPointersToList(ReferenceList &_list) : list(_list) {}

        virtual void visitPointer(Term* term) {
            if (term != NULL)
                list.appendUnique(term);
        }
    };

    AppendPointersToList visitor(result);
    visit_pointers(term, visitor);

    return result;
}

bool is_using(Term* user, Term* usee)
{

    return false;
}

void check_pointers(Term* term)
{
    ReferenceList pointers = list_all_pointers(term);

    for (unsigned int i=0; i < pointers.count(); i++) {
        if (is_bad_pointer(pointers[i])) {
            // todo
        }
    }
}

} // namespace circa
