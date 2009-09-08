// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

Term* find_named(Branch& branch, std::string const& name)
{
    assert(name != "");
    assert(name == "#out" || name[0] != '#'); // We shouldn't ever lookup hidden names

    if (branch.contains(name))
        return branch[name];

    Branch* outerScope = get_outer_scope(branch);

    if (outerScope == NULL)
        return get_global(name);
    else
        return find_named(*outerScope, name);
}

Term* get_named(Branch& branch, std::string const& name)
{
    if (branch.contains(name))
        return branch[name];
    else
        return NULL;
}

Term* get_dot_separated_name(Branch& branch, std::string const& name)
{
    int dotPos = -1;
    for (int i=0; name[i] != 0; i++) {
        if (name[i] == '.') {
            dotPos = i;
            break;
        }
    }

    if (dotPos == -1)
        return get_named(branch, name);

    if (dotPos == 0)
        return NULL;

    std::string head = name.substr(0, dotPos);
    std::string tail = name.substr(dotPos+1, name.length());

    if (!branch.contains(head))
        return NULL;

    Term* headTerm = branch[head];

    if (!is_branch(headTerm))
        return NULL;

    return get_dot_separated_name(as_branch(headTerm), tail);
}

Branch* get_parent_branch(Branch& branch)
{
    if (branch.owningTerm == NULL)
        return NULL;

    if (branch.owningTerm->owningBranch == NULL)
        return NULL;

    return branch.owningTerm->owningBranch;
}

Term* get_parent_term(Term* term)
{
    if (term->owningBranch == NULL)
        return NULL;
    if (term->owningBranch->owningTerm == NULL)
        return NULL;

    return term->owningBranch->owningTerm;
}

bool name_is_reachable_from(Term* term, Branch& branch)
{
    if (term->owningBranch == &branch)
        return true;

    Branch* parent = get_parent_branch(branch);

    if (parent == NULL)
        return false;

    return name_is_reachable_from(term, *parent);
}

// Returns whether or not we succeeded
bool get_relative_name_recursive(Branch& branch, Term* term, std::stringstream& output)
{
    if (name_is_reachable_from(term, branch)) {
        output << term->name;
        return true;
    }

    Term* parentTerm = get_parent_term(term);

    if (parentTerm == NULL)
        return false;

    bool success = get_relative_name_recursive(branch, parentTerm, output);

    if (success) {
        output << "." << term->name;
        return true;
    } else {
        return false;
    }
}

std::string get_relative_name(Branch& branch, Term* term)
{
    if (name_is_reachable_from(term, branch))
        return term->name;

    // Build a dot-separated name
    std::stringstream result;

    get_relative_name_recursive(branch, term, result);

    return result.str();
}

void expose_all_names(Branch& source, Branch& destination)
{
    for (TermNamespace::iterator it = source.names.begin(); it != source.names.end(); ++it)
    {
        std::string const& name = it->first;
        Term* term = it->second;
        if (name == "") continue;
        if (name[0] == '#' && name != "#out") continue;

        destination.bindName(term, name);
    }
}

} // namespace circa
