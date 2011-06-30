// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "heap_debugging.h"
#include "if_block.h"
#include "source_repro.h"
#include "term.h"

#include "names.h"

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
        internal_error("Branch's outer scope is a circular reference");

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
    
    // 'name' can be a qualified name. Find the end of the first identifier.
    int separatorLoc = find_qualified_name_separator(name.c_str());
    bool qualifiedName = separatorLoc != -1;
    size_t nameEnd;
    if (separatorLoc < 0)
        nameEnd = strlen(name.c_str());
    else
        nameEnd = unsigned(separatorLoc);

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

    // Recursively search inside prefix. TODO: should do this without allocating
    // a new string.
    if (qualifiedName)
        return get_named(nested_contents(prefix),
            name.substr(nameEnd+1, name.length()));

    return NULL;
}

Term* get_named(Branch const& branch, const char* name)
{
    return get_named(branch, std::string(name));
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
    if (nested_contents(term).length() == 0)
        return false;
    if (term->boolPropOptional("exposesNames", false))
        return true;
    if (term->function == INCLUDE_FUNC)
        return true;
    return false;
}

Term* get_named_at(Branch& branch, int index, std::string const& name)
{
    for (int i=index - 1; i >= 0; i--) {
        Term* term = branch[i];
        if (term == NULL) continue;
        if (term->name == name)
            return term;

        if (term->nestedContents && exposes_nested_names(term))
        {
            Term* nested = nested_contents(term)[name];
            if (nested != NULL)
                return nested;
        }
    }

    // Look in outer scopes
    if (branch.owningTerm == NULL)
        return NULL;

    return get_named_at(branch.owningTerm, name);
}
Term* get_named_at(Term* location, std::string const& name)
{
    assert_valid_term(location);
    if (location->owningBranch == NULL) return NULL;
    return get_named_at(*location->owningBranch, location->index, name);
}

Term* get_global(std::string name)
{
    if (KERNEL->contains(name))
        return KERNEL->get(name);

    return NULL;
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

            rightParent = get_parent_branch(*rightParent);
        }

        leftParent = get_parent_branch(*leftParent);
        rightParent = right->owningBranch;
    }

    return NULL;
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

    Branch& branch = *term->owningBranch;

    bool updatedName = true;
    while (updatedName) {
        updatedName = false;

        for (int i = term->index-1; i >= 0; i--) {
            Term* other = branch[i];
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

void expose_all_names(Branch& source, Branch& destination)
{
    for (TermNamespace::iterator it = source.names.begin(); it != source.names.end(); ++it)
    {
        std::string const& name = it->first;
        Term* term = it->second;

        ca_assert(term != NULL);
        if (name == "") continue;
        if (name[0] == '#' && name != "#out") continue;

        destination.bindName(term, name);
    }
}

Term* find_from_unique_name(Branch& branch, const char* name)
{
    // O(n) search, maybe this should be made more efficient.

    for (int i=0; i < branch.length(); i++) {
        if (strcmp(get_unique_name(branch[i]), name) == 0) {
            return branch[i];
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

        if (searchTerm->owningBranch == &kernel())
            break;

        searchTerm = get_parent_term(searchTerm);

        if (searchTerm == NULL)
            return false;
    }

    // Construct a qualified name.
    std::stringstream out;

    for (size_t i = stack.size()-1; i >= 0; i--) {
        out << searchTerm->uniqueName.name;
        if (i > 0)
            out << ":";
    }
    name = out.str();
    return true;
}

Term* get_term_from_global_name(const char* name)
{
    return NULL;
}

} // namespace circa
