// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

Term* find_named(Branch const& branch, std::string const& name)
{
    ca_assert(name != "");
    ca_assert(name == "#out" || name[0] != '#'); // We shouldn't ever lookup hidden names

    Term* result = get_named(branch, name);
    if (result != NULL)
        return result;

    // Name not found in this branch, check the outer scope.
    Branch* outerScope = get_outer_scope(branch);

    if (outerScope == &branch)
        throw std::runtime_error("Branch's outer scope is a circular reference");

    if (outerScope == NULL)
        return get_global(name);
    else
        return find_named(*outerScope, name);
}

Term* get_named(Branch const& branch, std::string const& name)
{
    // First, check for an exact match
    TermNamespace::const_iterator it;
    it = branch.names.find(name);
    if (it != branch.names.end())
        return it->second;
    
    // 'name' can be a qualified name. Find the end of the first identifier, stopping
    // at the : character or the end of string.
    unsigned nameEnd = 0;
    for (; name[nameEnd] != 0; nameEnd++) {
        if (name[nameEnd] == ':')
            break;
    }

    // Find this name in branch's namespace
    Term* prefix = NULL;
    for (it = branch.names.begin(); it != branch.names.end(); ++it) {
        std::string const& namespaceName = it->first;
        if (strncmp(name.c_str(), namespaceName.c_str(), nameEnd) == 0
            && namespaceName.length() == nameEnd) {

            // We found the part before the first :
            prefix = it->second;
            break;
        }
    }

    // Give up if prefix not found
    if (prefix == NULL)
        return NULL;

    // Give up if prefix does not refer to a branch
    if (!is_branch(prefix))
        return NULL;

    // Recursively search inside prefix. Future: should do this without allocating
    // a new string.
    std::string suffix = name.substr(nameEnd+1, name.length());
    
    return get_named(prefix->nestedContents, suffix);
}

Branch* get_parent_branch(Branch& branch)
{
    if (&branch == KERNEL)
        return NULL;

    if (branch.owningTerm == NULL)
        return KERNEL;

    if (branch.owningTerm->owningBranch == NULL)
        return KERNEL;

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

    // Don't include the names of hidden branches
    if (is_hidden(parentTerm)) {
        output << term->name;
        return true;
    }

    bool success = get_relative_name_recursive(branch, parentTerm, output);

    if (success) {
        output << ":" << term->name;
        return true;
    } else {
        return false;
    }
}

std::string get_relative_name(Branch& branch, Term* term)
{
    ca_assert(term != NULL);

    if (name_is_reachable_from(term, branch))
        return term->name;

    // Build a dot-separated name
    std::stringstream result;

    get_relative_name_recursive(branch, term, result);

    return result.str();
}

std::string get_relative_name(Term* location, Term* term)
{
    if (location == NULL)
        return get_relative_name(*KERNEL, term);

    if (location->owningBranch == NULL)
        return term->name;
    else
        return get_relative_name(*location->owningBranch, term);
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
