// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "importing.h"
#include "list.h"
#include "selector.h"

namespace circa {

void selector_prepend(caValue* selector, caValue* element)
{
    copy(element, list_insert(selector, 0));
}

/*
void get_selector_from_getter_chain(Term* getter, caValue* selector)
{
    while (true) {
        // Don't write a setter for a getter that already has a name; if the term has
        // a name then it's not part of this lexpr.
        if (getter->name != "")
            return;

        circa::Value element
        if (getter->function == FUNCS.get_index) {
            selector_prepend(selector
            return apply(branch, FUNCS.set_index, TermList(getter->input(0),
                getter->input(1), desiredValue));
        } else if (getter->function == FUNCS.get_field) {
            return apply(branch, FUNCS.set_field, TermList(getter->input(0),
                getter->input(1), desiredValue));
        } else if (getter->function == FUNCS.dynamic_method) {
            Term* fieldName = create_string(branch, getter->stringProp("syntax:functionName", ""));
            return apply(branch, FUNCS.set_field, TermList(getter->input(0),
                fieldName, desiredValue));
        }

        getter = getter->input(0);
    }
}
*/

void evaluate_selector(caStack* stack)
{
    copy(circa_input(stack, 0), circa_output(stack, 0));
}

void selector_setup_funcs(Branch* kernel)
{
    import_function(kernel, evaluate_selector, "selector(any elements :multiple) -> Selector");
}

} // namespace circa
