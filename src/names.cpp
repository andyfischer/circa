// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "kernel.h"
#include "heap_debugging.h"
#include "if_block.h"
#include "source_repro.h"
#include "term.h"

#include "names.h"

namespace circa {

Term* find_name(Branch* branch, const char* name)
{
    if (branch == NULL)
        return get_global(name);

    Term* result = find_local_name(branch, name);
    if (result != NULL)
        return result;

    // Name not found in this branch, check the outer scope.
    Branch* outerScope = get_outer_scope(branch);

    // Avoid infinite loop
    if (outerScope == branch)
        internal_error("Branch's outer scope is a circular reference");

    return find_name(outerScope, name);
}

Term* find_local_name(Branch* branch, const char* name)
{
    if (branch == NULL)
        return NULL;

    // First, check for an exact match
    TermNamespace::const_iterator it;
    it = branch->names.find(name);
    if (it != branch->names.end())
        return it->second;

    // 'name' can be a qualified name. Find the end of the first identifier.
    int separatorLoc = find_qualified_name_separator(name);

    // Give up if no separator found
    if (separatorLoc == -1)
        return NULL;

    // Find the namespace term
    std::string namespaceName(name, separatorLoc);
    Term* prefix = find_local_name(branch, namespaceName.c_str());

    // Give up if prefix not found
    if (prefix == NULL)
        return NULL;

    // Recursively search inside the prefix for the qualified suffix.
    std::string suffix = name + separatorLoc + 1;
    return find_local_name(nested_contents(prefix), suffix.c_str());

    return NULL;
}

int find_qualified_name_separator(const char* name)
{
    for (int i=0; name[i] != 0; i++) {
        if (name[i] == ':' && name[i+1] != 0)
            return i;
    }
    return -1;
}

bool exposes_nested_names(Term* term)
{
    if (term->nestedContents == NULL)
        return false;
    if (nested_contents(term)->length() == 0)
        return false;
    if (term->boolPropOptional("exposesNames", false))
        return true;
    if (term->function == INCLUDE_FUNC)
        return true;
    return false;
}

Term* get_named_at(Branch* branch, int index, std::string const& name)
{
    for (int i=index - 1; i >= 0; i--) {
        Term* term = branch->get(i);
        if (term == NULL) continue;
        if (term->name == name)
            return term;

        if (term->nestedContents && exposes_nested_names(term))
        {
            Term* nested = nested_contents(term)->get(name);
            if (nested != NULL)
                return nested;
        }
    }

    // Look in outer scopes
    if (branch->owningTerm == NULL)
        return NULL;

    return get_named_at(branch->owningTerm, name);
}
Term* get_named_at(Term* location, std::string const& name)
{
    assert_valid_term(location);
    if (location->owningBranch == NULL) return NULL;
    return get_named_at(location->owningBranch, location->index, name);
}

Term* get_global(std::string name)
{
    if (KERNEL->contains(name))
        return KERNEL->get(name);

    return NULL;
}

Branch* get_parent_branch(Branch* branch)
{
    if (branch == KERNEL)
        return NULL;

    if (branch->owningTerm == NULL)
        return KERNEL;

    if (branch->owningTerm->owningBranch == NULL)
        return KERNEL;

    return branch->owningTerm->owningBranch;
}

Term* get_parent_term(Term* term)
{
    if (term->owningBranch == NULL)
        return NULL;
    if (term->owningBranch->owningTerm == NULL)
        return NULL;

    return term->owningBranch->owningTerm;
}

bool name_is_reachable_from(Term* term, Branch* branch)
{
    if (term->owningBranch == branch)
        return true;

    Branch* parent = get_parent_branch(branch);

    if (parent == NULL)
        return false;

    return name_is_reachable_from(term, parent);
}

Branch* find_first_common_branch(Term* left, Term* right)
{
    Branch* leftParent = left->owningBranch;
    Branch* rightParent = right->owningBranch;

    if (leftParent == NULL) return NULL;
    if (rightParent == NULL) return NULL;

    // Walk upwards from left term.
    while (leftParent != NULL && leftParent != KERNEL) {

        // Walk upwards from right term.
        while (rightParent != NULL && leftParent != KERNEL) {
            if (leftParent == rightParent)
                return leftParent;

            rightParent = get_parent_branch(rightParent);
        }

        leftParent = get_parent_branch(leftParent);
        rightParent = right->owningBranch;
    }

    return NULL;
}

// Returns whether or not we succeeded
bool get_relative_name_recursive(Branch* branch, Term* term, std::stringstream& output)
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

std::string get_relative_name(Branch* branch, Term* term)
{
    ca_assert(term != NULL);

    if (name_is_reachable_from(term, branch))
        return term->name;

    // Build a dot-separated name
    std::stringstream result;

    get_relative_name_recursive(branch, term, result);

    return result.str();
}

std::string get_relative_name_at(Term* location, Term* term)
{
    if (location == NULL)
        return get_relative_name(KERNEL, term);

    if (location->owningBranch == NULL)
        return term->name;
    else
        return get_relative_name(location->owningBranch, term);
}

void update_unique_name(Term* term)
{
    Term::UniqueName& name = term->uniqueName;

    if (term->owningBranch == NULL) {
        name.name = term->name;
        return;
    }

    name.base = term->name;

    if (name.base == "")
        name.base = "_" + term->function->name;

    name.name = name.base;
    name.ordinal = 0;

    // Look for a name collision. We might need to keep looping, if our generated name
    // collides with an existing name.

    Branch* branch = term->owningBranch;

    bool updatedName = true;
    while (updatedName) {
        updatedName = false;

        for (int i = term->index-1; i >= 0; i--) {
            Term* other = branch->get(i);
            if (other == NULL) continue;

            // If another term shares the same base, then make sure our ordinal is
            // higher. This turns some O(n) cases into O(1)
            if ((other->uniqueName.base == name.base)
                    && (other->uniqueName.ordinal >= name.ordinal)) {
                name.ordinal = other->uniqueName.ordinal + 1;
                updatedName = true;

            // If this name is already used, then just try the next ordinal. This
            // case results in more blind searching, but it's necessary to handle
            // the situation where a generated name is already taken.
            } else if (other->uniqueName.name == name.name) {
                name.ordinal++;
                updatedName = true;
            }

            if (updatedName) {
                char ordinalBuf[30];
                sprintf(ordinalBuf, "%d", name.ordinal);
                name.name = name.base + "_" + ordinalBuf;
                break;
            }
        }
    }
}

const char* get_unique_name(Term* term)
{
    return term->uniqueName.name.c_str();
}

void expose_all_names(Branch* source, Branch* destination)
{
    for (TermNamespace::iterator it = source->names.begin(); it != source->names.end(); ++it)
    {
        std::string const& name = it->first;
        Term* term = it->second;

        ca_assert(term != NULL);
        if (name == "") continue;
        if (name[0] == '#' && name != "#out") continue;

        destination->bindName(term, name);
    }
}

Term* find_from_unique_name(Branch* branch, const char* name)
{
    // O(n) search, maybe this should be made more efficient.

    for (int i=0; i < branch->length(); i++) {
        if (strcmp(get_unique_name(branch->get(i)), name) == 0) {
            return branch->get(i);
        }
    }
    return NULL;
}

bool find_global_name(Term* term, std::string& name)
{
    // Search upwards, check if this term even has a global name.
    Term* searchTerm = term;

    std::vector<Term*> stack;

    while (true) {
        stack.push_back(searchTerm);

        if (searchTerm->owningBranch == kernel())
            break;

        searchTerm = get_parent_term(searchTerm);

        if (searchTerm == NULL)
            return false;
    }

    // Construct a qualified name.
    std::stringstream out;

    for (int i = stack.size()-1; i >= 0; i--) {
        out << stack[i]->uniqueName.name;
        if (i > 0)
            out << ":";
    }
    name = out.str();
    return true;
}
std::string find_global_name(Term* term)
{
    std::string out;
    find_global_name(term, out);
    return out;
}

Term* find_term_from_global_name_recr(Branch* searchBranch, const char* name)
{
    int separator = find_qualified_name_separator(name);
    
    if (separator == -1)
        return find_from_unique_name(searchBranch, name);

    std::string namePortion = std::string(name, separator);
    
    Term* searchTerm = find_from_unique_name(searchBranch, namePortion.c_str());
    if (searchTerm == NULL)
        return NULL;
    if (searchTerm->nestedContents == NULL)
        return NULL;

    return find_term_from_global_name_recr(searchTerm->nestedContents,
            &name[separator+1]);
}

Term* find_term_from_global_name(const char* name)
{
    Branch* searchBranch = kernel();
    return find_term_from_global_name_recr(searchBranch, name);
}

} // namespace circa
