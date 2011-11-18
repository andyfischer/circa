// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "kernel.h"
#include "function.h"
#include "if_block.h"
#include "introspection.h"
#include "source_repro.h"
#include "subroutine.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"

namespace circa {

bool is_declared_state(Term* term)
{
    return term->function == DECLARED_STATE_FUNC;
}

bool has_implicit_state(Term* term)
{
    return has_state_input(term_get_function_details(term));
}

bool is_function_stateful(Term* func)
{
    if (!is_function(func))
        return false;
    Function* attrs = as_function(func);
    if (attrs == NULL)
        return false;

    return function_has_state_input(attrs);
}

void on_stateful_function_call_created(Term* call)
{
    Branch* branch = call->owningBranch;

    // check if there is already a stateful input
    Term* existing = find_state_input(branch);
    if (existing != NULL)
        return;

    // None yet, insert one
    insert_state_input(branch);
}

void pack_any_open_state_vars(Branch* branch)
{
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;
        if (term->function == DECLARED_STATE_FUNC) {
            Term* result = branch->get(term->name);
            Term* pack = apply(branch, PACK_STATE_FUNC, TermList(
                find_open_state_result(branch, branch->length()),
                result));
            pack->setStringProp("field", unique_name(term));
            branch->move(pack, result->index + 1);
        }
    }
}

void get_type_from_branches_stateful_terms(Branch* branch, Branch* type)
{
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);

        if (!is_declared_state(term))
            continue;

        create_value(type, term->type, term->name);
    }
}

void get_state_description(Term* term, TaggedValue* output)
{
    if (term->function == FOR_FUNC) {
        List& list = *List::cast(output, 2);
        describe_state_shape(nested_contents(term), list[0]);
        copy(&RepeatSymbol, list[1]);
    } else if (term->function == IF_BLOCK_FUNC) {
        int numBranches = if_block_count_cases(term);

        List& list = *List::cast(output, numBranches);

        for (int bindex=0; bindex < numBranches; bindex++) {
            Branch* branch = nested_contents(if_block_get_case(term, bindex));
            describe_state_shape(branch, list[bindex]);
        }
    } else if (is_declared_state(term)) {
        set_string(output, declared_type(term)->name);
    } else if (is_function_stateful(term->function)) {
        describe_state_shape(nested_contents(term->function), output);
    }
}

void describe_state_shape(Branch* branch, TaggedValue* output)
{
    // Start off with an empty result
    set_null(output);

    Dict* dict = NULL;

    // Iterate through 'branch' and see if we find anything.
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);

        if (has_implicit_state(term) || is_declared_state(term)) {
            if (dict == NULL)
                dict = Dict::cast(output);

            TaggedValue* stateDescription = dict->insert(get_unique_name(term));
            get_state_description(term, stateDescription);
        }
    }
}

void strip_orphaned_state(TaggedValue* description, TaggedValue* state,
    TaggedValue* trash)
{
    set_null(trash);

    // If description is null then delete everything.
    if (is_null(description)) {
        swap(state, trash);
        set_null(state);
        return;
    }

    // If state is null, then there's nothing to do.
    if (is_null(state))
        return;

    // Handle a type name
    if (is_string(description)) {
        if ((state->value_type->name != as_string(description))
                && (as_string(description) != "any")) {
            swap(state, trash);
            set_null(state);
        }
        return;
    }

    // Handle a dictionary value
    if (is_dict(description)) {
        if (!is_dict(state)) {
            swap(state, trash);
            set_null(state);
            return;
        }

        Dict& descriptionDict = *Dict::checkCast(description);
        Dict& stateDict = *Dict::checkCast(state);
        Dict* trashDict = NULL;

        TaggedValue it;
        for (stateDict.iteratorStart(&it); !stateDict.iteratorFinished(&it);
                stateDict.iteratorNext(&it)) {

            const char* dictKey;
            TaggedValue* dictTaggedValue;

            stateDict.iteratorGet(&it, &dictKey, &dictTaggedValue);

            // Check if this key name doesn't exist in the description.
            if (!descriptionDict.contains(dictKey)) {

                if (trashDict == NULL)
                    trashDict = make_dict(trash);

                swap(dictTaggedValue, trashDict->insert(dictKey));

                stateDict.iteratorDelete(&it);
                continue;
            }

            // This key does exist in the description, recursively call to check
            // the key's value.
            
            TaggedValue nestedTrash;
            strip_orphaned_state(descriptionDict.get(dictKey), dictTaggedValue, &nestedTrash);

            if (!is_null(&nestedTrash)) {
                if (trashDict == NULL)
                    trashDict = make_dict(trash);
                swap(&nestedTrash, trashDict->insert(dictKey));
            }
        }
        return;
    }

    // Handle a list
    if (is_list(description)) {

        if (!is_list(state)) {
            swap(state, trash);
            set_null(state);
            return;
        }

        List& descriptionList = *List::checkCast(description);
        List& stateList = *List::checkCast(state);
        List* trashList = NULL;

        int descriptionIndex = 0;
        for (int index=0; index < stateList.length(); index++) {

            TaggedValue* stateTaggedValue = stateList[index];

            // Check if this state list is too long
            if (descriptionIndex >= descriptionList.length()) {
                if (trashList == NULL)
                    trashList = List::cast(trash, stateList.length());

                swap(stateTaggedValue, trashList->get(index));
                set_null(stateTaggedValue);
                return;
            }

            // check for :repeat symbol
            if (is_symbol(descriptionList[descriptionIndex]))
                descriptionIndex--;

            TaggedValue* descriptionTaggedValue = descriptionList[descriptionIndex];

            TaggedValue nestedTrash;
            strip_orphaned_state(descriptionTaggedValue, stateTaggedValue, &nestedTrash);

            if (!is_null(&nestedTrash)) {
                if (trashList == NULL)
                    trashList = List::cast(trash, stateList.length());

                swap(&nestedTrash, trashList->get(index));
            }
        }
        return;
    }

    internal_error("Unrecognized state description: " + description->toString());
}

void strip_orphaned_state(Branch* branch, TaggedValue* state, TaggedValue* trash)
{
    TaggedValue description;
    describe_state_shape(branch, &description);
    strip_orphaned_state(&description, state, trash);
}

void strip_orphaned_state(Branch* branch, TaggedValue* state)
{
    TaggedValue description;
    TaggedValue trash;
    describe_state_shape(branch, &description);
    strip_orphaned_state(&description, state, &trash);
}

} // namespace circa
