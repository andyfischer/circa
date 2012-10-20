// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "debug.h"
#include "kernel.h"
#include "heap_debugging.h"
#include "if_block.h"
#include "names_builtin.h"
#include "source_repro.h"
#include "string_type.h"
#include "term.h"
#include "world.h"

#include "names.h"

namespace circa {

struct RuntimeName
{
    // A RuntimeName is a name created dynamically at runtime (as opposed to a builtin
    // name, which is predeclared in names_builtin.h).

    char* str;
    Name namespaceFirst;
    Name namespaceRightRemainder;
};

const int c_FirstRuntimeName = name_LastBuiltinName + 1;
const int c_maxRuntimeNames = 2000;

RuntimeName g_runtimeNames[c_maxRuntimeNames];
int g_nextFreeNameIndex = 0;
std::map<std::string,Name> g_stringToSymbol;

// run_name_search: takes a NameSearch object and actually performs the search.
// There are many variations of find_name and find_local_name which all just wrap
// around this function.
Term* run_name_search(NameSearch* params);
bool exposes_nested_names(Term* term);

bool fits_lookup_type(Term* term, Name type)
{
    switch (type) {
        case name_LookupAny:
            return true;
        case name_LookupType:
            return is_type(term);
        case name_LookupFunction:
            return is_function(term);
        case name_LookupModule:
            return (term->function == FUNCS.imported_file);
    }
    internal_error("unknown type in fits_lookup_type");
    return false;
}

Term* run_name_search(NameSearch* params)
{
    if (params->name == 0)
        return NULL;

    Branch* branch = params->branch;

    if (branch == NULL)
        return NULL;

    // position can be -1, meaning 'last term'
    int position = params->position;
    if (position == -1)
        position = branch->length();

    if (position > branch->length())
        position = branch->length();

    // Look for an exact match.
    for (int i = position - 1; i >= 0; i--) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;

        if (term->nameSymbol == params->name
                && fits_lookup_type(term, params->lookupType)
                && (params->ordinal == -1 || term->uniqueOrdinal == params->ordinal))
            return term;

        // If this term exposes its names, then search inside the nested branch.
        if (term->nestedContents != NULL && exposes_nested_names(term)) {
            NameSearch nestedSearch;
            nestedSearch.branch = term->nestedContents;
            nestedSearch.name = params->name;
            nestedSearch.position = -1;
            nestedSearch.ordinal = -1;
            nestedSearch.lookupType = params->lookupType;
            nestedSearch.searchParent = false;
            Term* nested = run_name_search(&nestedSearch);
            if (nested != NULL)
                return nested;
        }
    }

    // Check if the name is a qualified name.
    Name namespacePrefix = qualified_name_get_first_section(params->name);

    // If it's a qualified name, search for the first prefix.
    if (namespacePrefix != name_None) {
        NameSearch nsSearch;
        nsSearch.branch = params->branch;
        nsSearch.name = namespacePrefix;
        nsSearch.position = params->position;
        nsSearch.ordinal = params->ordinal;
        nsSearch.lookupType = params->lookupType;
        nsSearch.searchParent = false;
        Term* nsPrefixTerm = run_name_search(&nsSearch);

        if (nsPrefixTerm != NULL) {
            // Recursively search inside the prefix for the remainder of the name.
            NameSearch nestedSearch;
            nestedSearch.branch = nested_contents(nsPrefixTerm);
            nestedSearch.name = qualified_name_get_remainder_after_first_section(params->name);
            nestedSearch.position = -1;
            nestedSearch.ordinal = params->ordinal;
            nestedSearch.lookupType = params->lookupType;
            nestedSearch.searchParent = false;
            return run_name_search(&nestedSearch);
        }
    }

    // Possibly continue search to parent branch.
    if (params->searchParent) {

        // Don't continue the search if we just searched the root.
        if (branch == global_root_branch())
            return NULL;

        // Search parent

        // the position is our parent's position plus one, so that we do look
        // at the parent's branch (in case our name has a namespace prefix
        // that refers back to the inside of this branch).
        //
        // TODO: This has an issue where, if we do have a qualified name that
        // refers back to this branch as a namespace, the search location will
        // be wrong later.
        //
        // Say we have branch:
        // namespace ns {
        //   a = 1
        //   b = 2  <-- search location
        //   c = 3
        // }
        //
        // We start at location 'b' and search for ns:c. This will fail at first, then
        // go to the parent branch which finds 'ns', which searches inside 'ns' to find 'c'.
        // However, ns:c is not visible at location 'b'.
        
        NameSearch parentSearch;

        Term* parentTerm = branch->owningTerm;
        if (parentTerm != NULL) {
            parentSearch.branch = parentTerm->owningBranch;
            parentSearch.position = parentTerm->index + 1;
        } else {
            parentSearch.branch = global_root_branch();
            parentSearch.position = -1;
        }
        parentSearch.name = params->name;
        parentSearch.lookupType = params->lookupType;
        parentSearch.ordinal = -1;
        parentSearch.searchParent = true;
        return run_name_search(&parentSearch);
    }

    return NULL;
}

Term* find_name(Branch* branch, Name name, int position, Name lookupType)
{
    NameSearch nameSearch;
    nameSearch.branch = branch;
    nameSearch.name = name;
    nameSearch.position = position;
    nameSearch.ordinal = -1;
    nameSearch.lookupType = lookupType;
    nameSearch.searchParent = true;
    return run_name_search(&nameSearch);
}

Term* find_local_name(Branch* branch, Name name, int position, Name lookupType)
{
    NameSearch nameSearch;
    nameSearch.branch = branch;
    nameSearch.name = name;
    nameSearch.position = position;
    nameSearch.ordinal = -1;
    nameSearch.lookupType = lookupType;
    nameSearch.searchParent = false;
    return run_name_search(&nameSearch);
}

Term* find_name(Branch* branch, const char* nameStr, int position, Name lookupType)
{
    Name name = name_from_string(nameStr);
    if (name == name_None)
        return NULL;

    NameSearch nameSearch;
    nameSearch.branch = branch;
    nameSearch.name = name;
    nameSearch.position = position;
    nameSearch.ordinal = -1;
    nameSearch.lookupType = lookupType;
    nameSearch.searchParent = true;
    return run_name_search(&nameSearch);
}

Term* find_local_name(Branch* branch, const char* nameStr, int position, Name lookupType)
{
    Name name = name_from_string(nameStr);
    if (name == name_None)
        return NULL;

    NameSearch nameSearch;
    nameSearch.branch = branch;
    nameSearch.name = name;
    nameSearch.position = position;
    nameSearch.ordinal = -1;
    nameSearch.lookupType = lookupType;
    nameSearch.searchParent = false;
    return run_name_search(&nameSearch);
}

void get_global_name(Term* term, caValue* nameOut)
{
    // Walk upwards, find all terms along the way.
    Term* searchTerm = term;

    std::vector<Term*> stack;

    while (true) {
        stack.push_back(searchTerm);

        if (searchTerm->owningBranch == global_root_branch())
            break;

        searchTerm = get_parent_term(searchTerm);

        if (searchTerm == NULL) {
            // Parent is NULL but we haven't yet reached the global root. This is
            // a deprecated style of branch that isn't connected to root. No global
            // name is possible.
            set_name(nameOut, name_None);
            return;
        }
    }

    // Construct a global qualified name.
    set_string(nameOut, "");
    for (int i = stack.size()-1; i >= 0; i--) {
        Term* subTerm = stack[i];

        // If this term has no name then we can't construct a global name. Bail out.
        if (subTerm->nameSymbol == name_None) {
            set_name(nameOut, name_None);
            return;
        }

        string_append(nameOut, name_to_string(subTerm->nameSymbol));

        if (subTerm->uniqueOrdinal != 0) {
            string_append(nameOut, "#");
            string_append(nameOut, subTerm->uniqueOrdinal);
        }

        if (i > 0)
            string_append(nameOut, ":");
    }
}

Term* find_from_global_name(World* world, const char* globalName)
{
    Branch* branch = world->root;

    // Loop, walking down the (possibly) qualified name.
    for (int step=0;; step++) {

        ca_assert(branch != NULL);

        int separatorPos = name_find_qualified_separator(globalName);
        int nameEnd = separatorPos;
        int ordinal = name_find_ordinal_suffix(globalName, &nameEnd);

        NameSearch nameSearch;
        nameSearch.name = existing_name_from_string(globalName, nameEnd);
        nameSearch.branch = branch;
        nameSearch.position = -1;
        nameSearch.ordinal = ordinal;
        nameSearch.lookupType = name_LookupAny;
        nameSearch.searchParent = false;

        Term* foundTerm = run_name_search(&nameSearch);

        // Stop if this name wasn't found.
        if (foundTerm == NULL)
            return NULL;

        // Stop if there's no more qualified sections.
        if (separatorPos == -1)
            return foundTerm;

        // Otherwise, continue the search.
        globalName = globalName + separatorPos + 1;
        branch = nested_contents(foundTerm);
    }
}

Term* find_name_at(Term* term, const char* nameStr)
{
    Name name = name_from_string(nameStr);
    if (name == name_None)
        return NULL;

    NameSearch nameSearch;
    nameSearch.branch = term->owningBranch;
    nameSearch.name = name;
    nameSearch.position = term->index;
    nameSearch.ordinal = -1;
    nameSearch.lookupType = name_LookupAny;
    nameSearch.searchParent = true;
    return run_name_search(&nameSearch);
}

Term* find_name_at(Term* term, Name name)
{
    NameSearch nameSearch;
    nameSearch.branch = term->owningBranch;
    nameSearch.name = name;
    nameSearch.position = term->index;
    nameSearch.ordinal = -1;
    nameSearch.lookupType = name_LookupAny;
    nameSearch.searchParent = true;
    return run_name_search(&nameSearch);
}

int name_find_qualified_separator(const char* name)
{
    for (int i=0; name[i] != 0; i++) {
        if (name[i] == ':' && name[i+1] != 0)
            return i;
    }
    return -1;
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int name_find_ordinal_suffix(const char* name, int* endPos)
{
    int originalEndPos = *endPos;

    // Walk backwards, see if this name even has an ordinal suffix.
    int search = *endPos - 1;
    if (*endPos == -1)
        search = strlen(name) - 1;

    bool foundADigit = false;
    bool foundOrdinalSuffix = false;

    while (true) {
        if (search < 0)
            break;

        if (is_digit(name[search])) {
            foundADigit = true;
            search--;
            continue;
        }

        if (foundADigit && name[search] == '#') {
            // Found a suffix of the form #123.
            foundOrdinalSuffix = true;
            *endPos = search;
            break;
        }

        break;
    }

    if (!foundOrdinalSuffix)
        return -1;

    // Parse and return the ordinal number.
    return atoi(name + *endPos + 1);
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
    return find_name(global_root_branch(), name);
}

Term* get_global(const char* nameStr)
{
    Name name = name_from_string(nameStr);
    if (name == name_None)
        return NULL;

    return get_global(name);
}

Branch* get_parent_branch(Branch* branch)
{
    if (branch == global_root_branch())
        return NULL;

    if (branch->owningTerm == NULL)
        return global_root_branch();

    if (branch->owningTerm->owningBranch == NULL)
        return global_root_branch();

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
    while (leftParent != NULL && leftParent != global_root_branch()) {

        // Walk upwards from right term.
        while (rightParent != NULL && leftParent != global_root_branch()) {
            if (leftParent == rightParent)
                return leftParent;

            rightParent = get_parent_branch(rightParent);
        }

        leftParent = get_parent_branch(leftParent);
        rightParent = right->owningBranch;
    }

    return NULL;
}

bool term_is_child_of_branch(Term* term, Branch* branch)
{
    while (term != NULL) {
        if (term->owningBranch == branch)
            return true;

        term = get_parent_term(term);
    }

    return false;
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
        return get_relative_name(global_root_branch(), term);

    if (location->owningBranch == NULL)
        return term->name;
    else
        return get_relative_name(location->owningBranch, term);
}

void get_relative_name_as_list(Term* term, Branch* relativeTo, caValue* nameOutput)
{
    set_list(nameOutput, 0);

    // Walk upwards and build the name, stop when we reach relativeTo.
    // The output list will be reversed but we'll fix that.

    while (true) {
        set_string(list_append(nameOutput), get_unique_name(term));

        if (term->owningBranch == relativeTo) {
            break;
        }

        term = get_parent_term(term);

        // If term is null, then it wasn't really a child of relativeTo
        if (term == NULL) {
            set_null(nameOutput);
            return;
        }
    }

    // Fix output list
    list_reverse(nameOutput);
}

Term* find_from_relative_name_list(caValue* name, Branch* relativeTo)
{
    if (is_null(name))
        return NULL;

    Term* term = NULL;
    for (int index=0; index < list_length(name); index++) {
        if (relativeTo == NULL)
            return NULL;

        term = find_from_unique_name(relativeTo, as_cstring(list_get(name, index)));

        if (term == NULL)
            return NULL;

        relativeTo = term->nestedContents;

        // relativeTo may now be NULL. But if we reached the end of this match, that's ok.
    }
    return term;
}

void update_unique_name(Term* term)
{
    Term::UniqueName& name = term->uniqueName;

    if (term->owningBranch == NULL) {
        name.name = term->name;
        return;
    }

    name.base = term->name;

    if (name.base == "") {
        if (term->function == NULL)
            name.base = "_anon";
        else
            name.base = "_" + term->function->name;
    }

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
    // O(n) search; this should be made more efficient.

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

bool name_is_valid(Name name)
{
    if (name < 0)
        return false;

    const char* builtin = builtin_name_to_string(name);
    if (builtin != NULL)
        return true;

    int runtimeIndex = name - c_FirstRuntimeName;
    if (runtimeIndex < 0 || runtimeIndex >= g_nextFreeNameIndex)
        return false;

    return true;
}

const char* name_to_string(Name name)
{
    const char* builtin = builtin_name_to_string(name);
    if (builtin != NULL)
        return builtin;

    // Runtime symbols
    if (name >= c_FirstRuntimeName)
        return g_runtimeNames[name - c_FirstRuntimeName].str;

    internal_error("Unknown name in name_to_string");
    return "";
}

void name_to_string(Name name, String* string)
{
    set_string((caValue*) string, name_to_string(name));
}

Name qualified_name_get_first_section(Name name)
{
    if (name < c_FirstRuntimeName)
        return 0;
    else
        return g_runtimeNames[name - c_FirstRuntimeName].namespaceFirst;
}

Name qualified_name_get_remainder_after_first_section(Name name)
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
    ca_assert(name_is_valid(val));
    set_null(tv);
    tv->value_type = &NAME_T;
    tv->value_data.asint = val;
}

Name existing_name_from_string(const char* str)
{
    INCREMENT_STAT(InternedNameLookup);

    std::map<std::string,Name>::const_iterator it;
    it = g_stringToSymbol.find(str);
    if (it != g_stringToSymbol.end())
        return it->second;

    return 0;
}

Name existing_name_from_string(const char* str, int len)
{
    INCREMENT_STAT(InternedNameLookup);

    std::string s;
    if (len == -1)
        s = str;
    else
        s = std::string(str, len);

    std::map<std::string,Name>::const_iterator it;
    it = g_stringToSymbol.find(s);
    if (it != g_stringToSymbol.end())
        return it->second;

    return 0;
}

// Runtime symbols
Name name_from_string(const char* str)
{
    // Empty string is name_None
    if (str[0] == 0)
        return name_None;

    // Check if name is already registered
    Name existing = existing_name_from_string(str);
    if (existing != name_None)
        return existing;

    // Not yet registered; add it to the list.
    INCREMENT_STAT(InternedNameCreate);

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
            // Create a temporary string to hold the substring before :
            char* tempstr = (char*) malloc(i + 1);
            memcpy(tempstr, str, i);
            tempstr[i] = 0;

            g_runtimeNames[index].namespaceFirst = name_from_string(tempstr);
            g_runtimeNames[index].namespaceRightRemainder = name_from_string(str + i + 1);
            free(tempstr);
            break;
        }
    }

    return name;
}
Name name_from_string(std::string const& str)
{
    return name_from_string(str.c_str());
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

extern "C" caName circa_to_name(const char* str)
{
    return circa::name_from_string(str);
}
extern "C" const char* circa_name_to_string(caName name)
{
    return circa::name_to_string(name);
}
