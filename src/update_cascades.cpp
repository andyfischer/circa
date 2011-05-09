// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "debug_valid_objects.h"
#include "function.h"
#include "tagged_value.h"
#include "term.h"
#include "update_cascades.h"
#include "types/list.h"

namespace circa {

const int INPUTS_CHANGED = 0x1;

static void on_repairable_link(Term* term, List& brokenLinks);

List& initialize_pending_update_item(Term* term)
{
    assert_valid_branch(term->owningBranch);

    Branch& branch = *term->owningBranch;
    List* pendingUpdates = NULL;

    // Cast or resize branch.pendingUpdates
    if (is_null(&branch.pendingUpdates))
        pendingUpdates = List::cast(&branch.pendingUpdates, branch.length());
    else {
        pendingUpdates = List::checkCast(&branch.pendingUpdates);
        pendingUpdates->resize(branch.length());
    }

    // Initialize the element at 'term'
    TaggedValue* item = pendingUpdates->get(term->index);
    if (is_null(item)) {
        List* itemList = List::cast(item, 2);
        set_int(itemList->get(0), 0);
    }

    return *List::checkCast(item);
}

void mark_inputs_changed(Term* term)
{
    List& item = initialize_pending_update_item(term);
    set_int(item[0], as_int(item[0]) | INPUTS_CHANGED);
}

void mark_repairable_link(Term* term, std::string const& name, int dependencyIndex)
{
    List& item = initialize_pending_update_item(term);
    List& brokenLinks = *List::lazyCast(item[1]);
    List& link = *List::cast(brokenLinks.append(), 2);
    set_string(link[0], name);
    set_int(link[1], dependencyIndex);
}

void finish_update_cascade(Branch& branch)
{
    if (is_null(&branch.pendingUpdates))
        return;

    ca_assert(!branch.currentlyCascadingUpdates);

    branch.currentlyCascadingUpdates = true;

    List& pendingUpdates = *List::checkCast(&branch.pendingUpdates);
    for (int index=0; index < pendingUpdates.length(); index++) {

        if (index >= branch.length())
            continue;

        Term* term = branch[index];
        TaggedValue* item = pendingUpdates[index];
        if (is_null(item))
            continue;
        List& itemList = *List::checkCast(item);
        int itemEnum = as_int(itemList[0]);
        if (itemEnum & INPUTS_CHANGED)
            on_inputs_changed(term);

        List* brokenLinks = List::checkCast(itemList[1]);
        if (brokenLinks != NULL)
            on_repairable_link(term, *brokenLinks);

        set_null(item);
    }
    
    // Make sure pendingUpdates is now all null. If it's not, then an update
    // has caused a change to an earlier term, which is bad.
    for (int index=0; index < pendingUpdates.length(); index++) {
        ca_assert(is_null(pendingUpdates[index]));
    }

    set_null(&branch.pendingUpdates);
    branch.currentlyCascadingUpdates = false;
}

void on_inputs_changed(Term* term)
{
    FunctionAttrs* attrs = get_function_attrs(term->function);
    if (attrs == NULL)
        return;

    FunctionAttrs::PostInputChange func = attrs->postInputChange;

    if (func)
        func(term);
}

void on_repairable_link(Term* term, List& brokenLinks)
{
    for (int i=0; i < brokenLinks.length(); i++) {
        List& linkInfo = *List::checkCast(brokenLinks[i]);
        int dependencyIndex = as_int(linkInfo[1]);

        if (term->dependency(dependencyIndex) != NULL)
            continue;

        Term* fixedLink = get_named_at(term, as_string(linkInfo[0]));
        term->setDependency(dependencyIndex, fixedLink);
    }
}

} // namespace circa
