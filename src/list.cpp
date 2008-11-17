// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "cpp_interface.h"
#include "importing.h"
#include "list.h"
#include "runtime.h"
#include "type.h"
#include "values.h"

namespace circa {

List::List(List const& copy)
{
    for (int i=0; i < copy.count(); i++) {
        Term* term = appendSlot(copy[i]->type);
        duplicate_value(copy[i], term);
    }
}

Term*
List::append(Term* term)
{
    Term* newTerm = create_var(&this->branch, term->type);
    recycle_value(term, newTerm);
    this->items.append(newTerm);
    return newTerm;
}

Term*
List::appendSlot(Term* type)
{
    Term* newTerm = create_var(&this->branch, type);
    this->items.append(newTerm);
    return newTerm;
}

std::string List__toString(Term* caller)
{
    std::stringstream out;
    List& list = as_list(caller);
    out << "[";
    bool first_item = true;
    for (int i=0; i < list.count(); i++) {
        if (!first_item) out << ", ";
        out << list[i]->toString();
        first_item = false;
    }
    out << "]";
    return out.str();
}

bool is_list(Term* term)
{
    return term->type == LIST_TYPE;
}

List& as_list(Term* term)
{
    assert_type(term, LIST_TYPE);
    assert(term->value != NULL);
    return *((List*) term->value);
}

List& as_list_unchecked(Term* term)
{
    assert(term->value != NULL);
    return *((List*) term->value);
}

ReferenceList List::toReferenceList() const
{
    ReferenceList result;
    for (int i=0; i < count(); i++) {
        result.append(get(i)->asRef());
    }
    return result;
}

namespace pack_list {

    void evaluate(Term* caller) {
        as_list(caller).clear();

        for (unsigned int i=0; i < caller->inputs.count(); i++) {
            as_list(caller).append(caller->inputs[i]);
        }
    }
}

namespace get_list_references {

    void evaluate(Term* caller) {
        Term* input = caller->inputs[0];
        if (as_function(input->function).name != "pack-list") {
            error_occured(caller,
                    "get-list-refernces only works with pack-list as input");
            return;
        }

        List& caller_list = as_list(caller);
        caller_list.clear();

        for (unsigned int i=0; i < input->inputs.count(); i++) {
            caller_list.appendSlot(REFERENCE_TYPE)->asRef() = input->inputs[i];
        }
    }
}

void initialize_list_functions(Branch* kernel)
{
    Term* pack_list = quick_create_function(kernel, "pack-list",
            pack_list::evaluate, ReferenceList(ANY_TYPE), LIST_TYPE);
    as_function(pack_list).variableArgs = true;
    kernel->bindName(pack_list, "list");
    Term* get_list_references = quick_create_function(kernel, "get-list-references",
        get_list_references::evaluate, ReferenceList(LIST_TYPE), LIST_TYPE);
    as_function(get_list_references).meta = true;
}

} // namespace circa
