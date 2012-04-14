// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "kernel.h"
#include "heap_debugging.h"
#include "if_block.h"
#include "source_repro.h"
#include "term.h"

#include "names.h"

namespace circa {

struct RuntimeName
{
    char* str;
    Name namespaceFirst;
    Name namespaceRightRemainder;
};

const int c_maxRuntimeNames = 2000;

RuntimeName g_runtimeNames[c_maxRuntimeNames];
int g_nextFreeNameIndex = 0;
std::map<std::string,Name> g_stringToSymbol;

bool exposes_nested_names(Term* term);

bool fits_lookup_type(Term* term, NameLookupType type)
{
    switch (type) {
        case NAME_LOOKUP_ANY: return true;
        case NAME_LOOKUP_TYPE: return is_type(term);
        case NAME_LOOKUP_FUNCTION: return is_function(term);
    }
    internal_error("");
    return false;
}

Term* find_local_name(Branch* branch, Name name, int location, NameLookupType lookupType)
{
    if (name == 0)
        return NULL;

    if (branch == NULL)
        return NULL;

    // location can be -1, meaning 'last term'
    if (location == -1)
        location = branch->length();

    // Look for an exact match.
    for (int i = location - 1; i >= 0; i--) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;

        if (term->nameSymbol == name && fits_lookup_type(term, lookupType))
            return term;

        // If this term exposes its names, then search inside the nested branch.
        if (term->nestedContents != NULL && exposes_nested_names(term)) {
            Term* nested = find_local_name(term->nestedContents, name, -1, lookupType);
            if (nested != NULL)
                return nested;
        }
    }

    // Check if the name is a qualified name.
    Name namespacePrefix = name_get_namespace_first(name);

    if (namespacePrefix == name_None)
        return NULL;

    Term* nsPrefixTerm = find_local_name(branch, namespacePrefix, location, lookupType);

    // Give up if prefix not found
    if (nsPrefixTerm == NULL)
        return NULL;

    // Recursively search inside the prefix for the qualified suffix.
    return find_local_name(nested_contents(nsPrefixTerm),
        name_get_namespace_rr(name), -1, lookupType);
}

Term* find_name(Branch* branch, Name name, int location, NameLookupType lookupType)
{
    if (name == 0)
        return NULL;

    if (branch == NULL) {
        branch = KERNEL;
    }

    Term* result = find_local_name(branch, name, location, lookupType);
    if (result != NULL)
        return result;

    // Name not found in this branch.

    // Don't continue the search if this is the kernel
    if (branch == KERNEL)
        return NULL;

    // Search parent
    Term* parent = branch->owningTerm;
    if (parent != NULL) {
        // find_name with the parent's location plus one, so that we do look
        // at the parent's branch (in case our name has a namespace prefix
        // that refers to this branch).
        return find_name(parent->owningBranch, name, parent->index + 1, lookupType);
    }

    // No parent, search kernel
    return get_global(name);
}

Term* find_name(Branch* branch, const char* nameStr, int location, NameLookupType lookupType)
{
    Name name = name_from_string(nameStr);
    return find_name(branch, name, location, lookupType);
}

// Finds a name in this branch.
Term* find_local_name(Branch* branch, const char* nameStr, int location, NameLookupType lookupType)
{
    Name name = name_from_string(nameStr);
    return find_local_name(branch, name, location, lookupType);
}

Term* find_name_at(Term* term, const char* name)
{
    return find_name(term->owningBranch, name, term->index);
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
    if (term->function == FUNCS.include_func)
        return true;
    if (term->function == FUNCS.imported_file)
        return true;

    return false;
}

Term* get_global(Name name)
{
    return find_name(KERNEL, name);
}

Term* get_global(const char* name)
{
    return get_global(name_from_string(name));
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
Term* get_parent_term(Term* term, int levels)
{
    for (int i=0; i < levels; i++) {
        term = get_parent_term(term);
        if (term == NULL)
            return NULL;
    }
    return term;
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

Term* find_from_unique_name(Branch* branch, const char* name)
{
    // O(n) search, maybe this should be made more efficient.

    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;
        if (strcmp(get_unique_name(term), name) == 0) {
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

const char* name_to_string(Name name)
{
    // Runtime symbols
    if (name >= c_FirstRuntimeName)
        return g_runtimeNames[name - c_FirstRuntimeName].str;

    // Builtin symbols
    switch (name) {
        case name_None: return "";
        case name_File: return "File";
        case name_Newline: return "Newline";
        case name_Out: return "Out";
        case name_Unknown: return "Unknown";
        case name_Repeat: return "Repeat";
        case name_Success: return "Success";
        case name_Failure: return "Failure";
        case name_FileNotFound: return "FileNotFound";
        case name_NotEnoughInputs: return "NotEnoughInputs";
        case name_TooManyInputs: return "TooManyInputs";
        case name_ExtraOutputNotFound: return "ExtraOutputNotFound";
        case name_Default: return "Default";
        case name_ByDemand: return "ByDemand";
        case name_Unevaluated: return "Unevaluated";
        case name_Consumed: return "Consumed";
        case name_InProgress: return "InProgress";
        case name_Lazy: return "Lazy";
    }

    internal_error("Unknown name in name_to_string");
    return "";
}

void name_to_string(Name name, String* string)
{
    set_string((caValue*) string, name_to_string(name));
}

Name name_get_namespace_first(Name name)
{
    if (name < c_FirstRuntimeName)
        return 0;
    else
        return g_runtimeNames[name - c_FirstRuntimeName].namespaceFirst;
}

Name name_get_namespace_rr(Name name)
{
    if (name < c_FirstRuntimeName)
        return 0;
    else
        return g_runtimeNames[name - c_FirstRuntimeName].namespaceRightRemainder;
}

Name as_name(caValue* tv)
{
    return tv->value_data.asint;
}

void set_name(caValue* tv, Name val)
{
    set_null(tv);
    tv->value_type = &NAME_T;
    tv->value_data.asint = val;
}

Name existing_name_from_string(const char* str)
{
    std::map<std::string,Name>::const_iterator it;
    it = g_stringToSymbol.find(str);
    if (it != g_stringToSymbol.end())
        return it->second;

    return 0;
}

// Runtime symbols
Name name_from_string(const char* str)
{
    // Check if name is already registered
    {
        Name name = existing_name_from_string(str);
        if (name != 0)
            return name;
    }

    // Not yet registered; add it to the list.
    Name index = g_nextFreeNameIndex++;
    g_runtimeNames[index].str = strdup(str);
    g_runtimeNames[index].namespaceFirst = 0;
    g_runtimeNames[index].namespaceRightRemainder = 0;
    Name name = index + c_FirstRuntimeName;
    g_stringToSymbol[str] = name;

    // Search the string for a : name, if found we'll update the name's
    // namespace links.
    int len = strlen(str);
    for (int i=0; i < len; i++) {
        if (str[i] == ':') {
            char* tempstr = strndup(str, i);
            g_runtimeNames[index].namespaceFirst = name_from_string(tempstr);
            g_runtimeNames[index].namespaceRightRemainder = name_from_string(str + i + 1);
            free(tempstr);
            break;
        }
    }

    return name;
}
Name name_from_string(caValue* str)
{
    return name_from_string(as_cstring(str));
}
void name_dealloc_global_data()
{
    for (int i=0; i < g_nextFreeNameIndex; i++)
        free(g_runtimeNames[i].str);
    g_nextFreeNameIndex = 0;
    g_stringToSymbol.clear();
}

} // namespace circa

extern "C" caName circa_name(const char* str)
{
    return circa::existing_name_from_string(str);
}
extern "C" const char* circa_name_to_string(caName name)
{
    return circa::name_to_string(name);
}
