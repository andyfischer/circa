// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "builtins.h"
#include "errors.h"
#include "function.h"
#include "if_block.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"

namespace circa {

bool is_get_state(Term* term)
{
    return term->function->name == "get_state_field";
}

bool has_implicit_state(Term* term)
{
    if (is_function_stateful(term->function))
        return true;
    if (has_any_inlined_state(term->nestedContents))
        return true;
    return false;
}

bool is_function_stateful(Term* func)
{
    if (!is_function(func))
        return false;
    FunctionAttrs* attrs = get_function_attrs(func);
    if (attrs == NULL)
        return false;
    Term* stateType = attrs->implicitStateType;
    return (stateType != NULL && stateType != VOID_TYPE);
}

bool has_any_inlined_state(Branch& branch)
{
    // This result is cached on the branch. Check if branch.hasInlinedState has
    // a valid value.
    if (is_bool(&branch.hasInlinedState))
        return as_bool(&branch.hasInlinedState);

    // No valid value, recalculate.
    bool result = false;
    for (int i=0; i < branch.length(); i++) {
        if (is_get_state(branch[i])) {
            result = true;
            break;
        }

        if (has_implicit_state(branch[i])) {
            result = true;
            break;
        }
    }

    set_bool(&branch.hasInlinedState, result);
    return result;
}

void mark_branch_as_having_inlined_state(Branch& branch)
{
    if (is_bool(&branch.hasInlinedState) && as_bool(&branch.hasInlinedState))
        return;

    set_bool(&branch.hasInlinedState, true);
    Branch* parent = get_parent_branch(branch);
    if (parent != NULL)
        mark_branch_as_having_inlined_state(*parent);
}

void mark_branch_as_possibly_not_having_inlined_state(Branch& branch)
{
    if (is_null(&branch.hasInlinedState))
        return;

    set_null(&branch.hasInlinedState);
    Branch* parent = get_parent_branch(branch);
    if (parent != NULL)
        mark_branch_as_possibly_not_having_inlined_state(*parent);
}

void get_type_from_branches_stateful_terms(Branch& branch, Branch& type)
{
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];

        if (!is_get_state(term))
            continue;

        create_value(type, term->type, term->name);
    }
}

void get_state_description(Term* term, TaggedValue* output)
{
    if (term->function == FOR_FUNC) {
        List& list = *List::cast(output, 2);
        describe_state_shape(term->nestedContents, list[0]);
        copy(&REPEAT_SYMBOL, list[1]);
    } else if (term->function == IF_BLOCK_FUNC) {
        int numBranches = if_block_num_branches(term);

        List& list = *List::cast(output, numBranches);

        for (int bindex=0; bindex < numBranches; bindex++) {
            Branch* branch = if_block_get_branch(term, bindex);
            describe_state_shape(*branch, list[bindex]);
        }
    } else if (is_get_state(term)) {
        set_string(output, declared_type(term)->name);
    }
}

void describe_state_shape(Branch& branch, TaggedValue* output)
{
    // Start off with an empty result
    set_null(output);

    Dict* dict = NULL;

    // Iterate through 'branch' and see if we find anything.
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];

        if (has_implicit_state(term) || is_get_state(term)) {
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

    // if description is null then delete everything
    if (is_null(description)) {
        swap(state, trash);
        set_null(state);
        return;
    }

    // if state is null, then there's nothing to do.
    if (is_null(state))
        return;

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
            TaggedValue* dictValue;

            stateDict.iteratorGet(&it, &dictKey, &dictValue);

            // Check if this key name doesn't exist in the description.
            if (!descriptionDict.contains(dictKey)) {

                if (trashDict == NULL)
                    trashDict = make_dict(trash);

                swap(dictValue, trashDict->insert(dictKey));

                stateDict.iteratorDelete(&it);
                continue;
            }

            // This key does exist in the description, recursively call to check
            // the key's value.
            
            TaggedValue nestedTrash;
            strip_orphaned_state(descriptionDict.get(dictKey), dictValue, &nestedTrash);

            if (!is_null(&nestedTrash)) {
                if (trashDict == NULL)
                    trashDict = make_dict(trash);
                swap(&nestedTrash, trashDict->insert(dictKey));
            }
        }
        return;
    }

    // Handle a type name
    if (is_string(description)) {
        if (state->value_type->name != as_string(description)) {
            swap(state, trash);
            set_null(state);
        }
        return;
    }

    internal_error("Unrecognized state description");
}

void strip_orphaned_state(Branch& branch, TaggedValue* state, TaggedValue* trash)
{
    TaggedValue description;
    describe_state_shape(branch, &description);
    strip_orphaned_state(&description, state, trash);
}

void strip_orphaned_state(Branch& branch, TaggedValue* state)
{
    TaggedValue description;
    TaggedValue trash;
    describe_state_shape(branch, &description);
    strip_orphaned_state(&description, state, &trash);
}

} // namespace circa
