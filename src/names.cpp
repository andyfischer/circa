// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "debug.h"
#include "function.h"
#include "kernel.h"
#include "heap_debugging.h"
#include "if_block.h"
#include "inspection.h"
#include "names_builtin.h"
#include "source_repro.h"
#include "string_type.h"
#include "term.h"
#include "world.h"

#include "names.h"

namespace circa {

// run_name_search: takes a NameSearch object and actually performs the search.
// There are many variations of find_name and find_local_name which all just wrap
// around this function.
Term* run_name_search(NameSearch* params);

bool exposes_nested_names(Term* term);

bool fits_lookup_type(Term* term, Symbol type)
{
    switch (type) {
        case sym_LookupAny:
            return true;
        case sym_LookupType:
            return is_type(term);
        case sym_LookupFunction:
            return is_function(term);
        case sym_LookupModule:
            return (term->function == FUNCS.module);
    }
    internal_error("unknown type in fits_lookup_type");
    return false;
}

Term* run_name_search(NameSearch* params)
{
    if (is_null(&params->name) || string_eq(&params->name, ""))
        return NULL;

    Block* block = params->block;

    if (block == NULL)
        return NULL;

    // position can be -1, meaning 'last term'
    int position = params->position;
    if (position == -1)
        position = block->length();

    if (position > block->length())
        position = block->length();

    // Look for an exact match.
    for (int i = position - 1; i >= 0; i--) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;

        if (equals(&term->nameValue, &params->name)
                && fits_lookup_type(term, params->lookupType)
                && (params->ordinal == -1 || term->uniqueOrdinal == params->ordinal))
            return term;

        // If this term exposes its names, then search inside the nested block.
        if (term->nestedContents != NULL && exposes_nested_names(term)) {
            NameSearch nestedSearch;
            nestedSearch.block = term->nestedContents;
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
    Value namespacePrefix;
    qualified_name_get_first_section(&params->name, &namespacePrefix);

    // If it's a qualified name, search for the first prefix.
    if (!is_null(&namespacePrefix)) {
        NameSearch nsSearch;
        nsSearch.block = params->block;
        nsSearch.name = namespacePrefix;
        nsSearch.position = params->position;
        nsSearch.ordinal = params->ordinal;
        nsSearch.lookupType = sym_LookupAny;
        nsSearch.searchParent = false;
        Term* nsPrefixTerm = run_name_search(&nsSearch);

        if (nsPrefixTerm != NULL) {
            // Recursively search inside the prefix for the remainder of the name.
            NameSearch nestedSearch;
            nestedSearch.block = nested_contents(nsPrefixTerm);
            qualified_name_get_remainder_after_first_section(&params->name, &nestedSearch.name);
            nestedSearch.position = -1;
            nestedSearch.ordinal = params->ordinal;
            nestedSearch.lookupType = params->lookupType;
            nestedSearch.searchParent = false;
            return run_name_search(&nestedSearch);
        }
    }

    // Possibly continue search to parent block.
    if (params->searchParent) {

        // Don't continue the search if we just searched the root.
        if (block == global_root_block())
            return NULL;

        // Search parent

        // the position is our parent's position plus one, so that we do look
        // at the parent's block (in case our name has a namespace prefix
        // that refers back to the inside of this block).
        //
        // TODO: This has an issue where, if we do have a qualified name that
        // refers back to this block as a namespace, the search location will
        // be wrong later.
        //
        // Say we have block:
        // namespace ns {
        //   a = 1
        //   b = 2  <-- search location
        //   c = 3
        // }
        //
        // We start at location 'b' and search for ns:c. This will fail at first, then
        // go to the parent block which finds 'ns', which searches inside 'ns' to find 'c'.
        // However, ns:c is not visible at location 'b'.
        
        NameSearch parentSearch;

        Term* parentTerm = block->owningTerm;
        if (parentTerm != NULL) {
            parentSearch.block = parentTerm->owningBlock;
            parentSearch.position = parentTerm->index + 1;
        } else {
            parentSearch.block = global_root_block();
            parentSearch.position = -1;
        }
        copy(&params->name, &parentSearch.name);
        parentSearch.name = params->name;
        parentSearch.lookupType = params->lookupType;
        parentSearch.ordinal = -1;
        parentSearch.searchParent = true;
        return run_name_search(&parentSearch);
    }

    return NULL;
}

Term* find_name(Block* block, caValue* name, int position, Symbol lookupType)
{
    NameSearch nameSearch;
    nameSearch.block = block;
    copy(name, &nameSearch.name);
    nameSearch.position = position;
    nameSearch.ordinal = -1;
    nameSearch.lookupType = lookupType;
    nameSearch.searchParent = true;
    return run_name_search(&nameSearch);
}

Term* find_local_name(Block* block, caValue* name, int position, Symbol lookupType)
{
    NameSearch nameSearch;
    nameSearch.block = block;
    copy(name, &nameSearch.name);
    nameSearch.position = position;
    nameSearch.ordinal = -1;
    nameSearch.lookupType = lookupType;
    nameSearch.searchParent = false;
    return run_name_search(&nameSearch);
}

Term* find_name(Block* block, const char* nameStr, int position, Symbol lookupType)
{
    NameSearch nameSearch;
    nameSearch.block = block;
    set_string(&nameSearch.name, nameStr);
    nameSearch.position = position;
    nameSearch.ordinal = -1;
    nameSearch.lookupType = lookupType;
    nameSearch.searchParent = true;
    return run_name_search(&nameSearch);
}

Term* find_local_name(Block* block, const char* nameStr, int position, Symbol lookupType)
{
    NameSearch nameSearch;
    nameSearch.block = block;
    set_string(&nameSearch.name, nameStr);
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

        if (searchTerm->owningBlock == global_root_block())
            break;

        searchTerm = get_parent_term(searchTerm);

        if (searchTerm == NULL) {
            // Parent is NULL but we haven't yet reached the global root. This is
            // a deprecated style of block that isn't connected to root. No global
            // name is possible.
            set_symbol(nameOut, sym_None);
            return;
        }
    }

    // Construct a global qualified name.
    set_string(nameOut, "");
    for (int i = stack.size()-1; i >= 0; i--) {
        Term* subTerm = stack[i];

        // If this term has no name then we can't construct a global name. Bail out.
        if (has_empty_name(subTerm)) {
            set_symbol(nameOut, sym_None);
            return;
        }

        string_append(nameOut, &subTerm->nameValue);

        if (subTerm->uniqueOrdinal != 0) {
            string_append(nameOut, "#");
            string_append(nameOut, subTerm->uniqueOrdinal);
        }

        if (i > 0)
            string_append(nameOut, ":");
    }
}

void get_global_name(Block* block, caValue* nameOut)
{
    if (block->owningTerm == NULL) {
        set_null(nameOut);
        return;
    }
    get_global_name(block->owningTerm, nameOut);
}

Term* find_from_global_name(World* world, const char* globalNameStr)
{
    Block* block = world->root;

    // Loop, walking down the (possibly) qualified name.
    for (int step=0;; step++) {

        Value globalName;
        set_string(&globalName, globalNameStr);

        ca_assert(block != NULL);

        int separatorPos = name_find_qualified_separator(globalNameStr);
        int nameEnd = separatorPos;
        int ordinal = name_find_ordinal_suffix(globalNameStr, &nameEnd);

        NameSearch nameSearch;
        string_slice(&globalName, 0, nameEnd, &nameSearch.name);
        nameSearch.block = block;
        nameSearch.position = -1;
        nameSearch.ordinal = ordinal;
        nameSearch.lookupType = sym_LookupAny;
        nameSearch.searchParent = false;

        Term* foundTerm = run_name_search(&nameSearch);

        // Stop if this name wasn't found.
        if (foundTerm == NULL)
            return NULL;

        // Stop if there's no more qualified sections.
        if (separatorPos == -1)
            return foundTerm;

        // Otherwise, continue the search.
        globalNameStr = globalNameStr + separatorPos + 1;
        block = nested_contents(foundTerm);
    }
}

Term* find_name_at(Term* term, const char* name)
{
    NameSearch nameSearch;
    nameSearch.block = term->owningBlock;
    set_string(&nameSearch.name, name);
    nameSearch.position = term->index;
    nameSearch.ordinal = -1;
    nameSearch.lookupType = sym_LookupAny;
    nameSearch.searchParent = true;
    return run_name_search(&nameSearch);
}

Term* find_name_at(Term* term, caValue* name)
{
    NameSearch nameSearch;
    nameSearch.block = term->owningBlock;
    copy(name, &nameSearch.name);
    nameSearch.position = term->index;
    nameSearch.ordinal = -1;
    nameSearch.lookupType = sym_LookupAny;
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
    // Walk backwards, see if this name even has an ordinal suffix.
    int search = *endPos - 1;
    if (*endPos == -1)
        search = (int) strlen(name) - 1;

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
    if (term->function == FUNCS.module)
        return true;

    return false;
}

Term* find_global(const char* nameStr)
{
    return find_name(global_root_block(), nameStr);
}

Block* get_parent_block(Block* block)
{
    if (block == global_root_block())
        return NULL;

    if (block->owningTerm == NULL)
        return global_root_block();

    if (block->owningTerm->owningBlock == NULL)
        return global_root_block();

    return block->owningTerm->owningBlock;
}

Term* get_parent_term(Term* term)
{
    if (term->owningBlock == NULL)
        return NULL;
    if (term->owningBlock->owningTerm == NULL)
        return NULL;

    return term->owningBlock->owningTerm;
}

Term* get_parent_term(Block* block)
{
    return block->owningTerm;
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

bool sym_is_reachable_from(Term* term, Block* block)
{
    if (term->owningBlock == block)
        return true;

    Block* parent = get_parent_block(block);

    if (parent == NULL)
        return false;

    return sym_is_reachable_from(term, parent);
}

Block* find_first_common_block(Term* left, Term* right)
{
    Block* leftParent = left->owningBlock;
    Block* rightParent = right->owningBlock;

    if (leftParent == NULL) return NULL;
    if (rightParent == NULL) return NULL;

    // Walk upwards from left term.
    while (leftParent != NULL && leftParent != global_root_block()) {

        // Walk upwards from right term.
        while (rightParent != NULL && leftParent != global_root_block()) {
            if (leftParent == rightParent)
                return leftParent;

            rightParent = get_parent_block(rightParent);
        }

        leftParent = get_parent_block(leftParent);
        rightParent = right->owningBlock;
    }

    return NULL;
}

bool term_is_child_of_block(Term* term, Block* block)
{
    while (term != NULL) {
        if (term->owningBlock == block)
            return true;

        term = get_parent_term(term);
    }

    return false;
}

// Returns whether or not we succeeded
bool get_relative_name_recursive(Block* block, Term* term, std::stringstream& output)
{
    if (sym_is_reachable_from(term, block)) {
        output << term->name;
        return true;
    }

    Term* parentTerm = get_parent_term(term);

    if (parentTerm == NULL)
        return false;

    // Don't include the names of hidden blocks
    if (is_hidden(parentTerm)) {
        output << term->name;
        return true;
    }

    bool success = get_relative_name_recursive(block, parentTerm, output);

    if (success) {
        output << ":" << term->name;
        return true;
    } else {
        return false;
    }
}

std::string get_relative_name(Block* block, Term* term)
{
    ca_assert(term != NULL);

    if (sym_is_reachable_from(term, block))
        return term->name;

    // Build a dot-separated name
    std::stringstream result;

    get_relative_name_recursive(block, term, result);

    return result.str();
}

std::string get_relative_name_at(Term* location, Term* term)
{
    if (location == NULL)
        return get_relative_name(global_root_block(), term);

    if (location->owningBlock == NULL)
        return term->name;
    else
        return get_relative_name(location->owningBlock, term);
}

void get_relative_name_as_list(Term* term, Block* relativeTo, caValue* nameOutput)
{
    set_list(nameOutput, 0);

    // Walk upwards and build the name, stop when we reach relativeTo.
    // The output list will be reversed but we'll fix that.

    while (true) {
        set_value(list_append(nameOutput), get_unique_name(term));

        if (term->owningBlock == relativeTo) {
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

Term* find_from_relative_name_list(caValue* name, Block* relativeTo)
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

    if (term->owningBlock == NULL) {
        copy(&term->nameValue, &name.name);
        return;
    }

    copy(&term->nameValue, &name.base);

    if (string_empty(&name.base)) {
        if (term->function == NULL)
            set_string(&name.base, "_anon");
        else {
            set_string(&name.base, "_");
            string_append(&name.base, &term->function->nameValue);
        }
    }

    copy(&name.base, &name.name);
    name.ordinal = 0;

    // Look for a name collision. We might need to keep looping, if our generated name
    // collides with an existing name.

    Block* block = term->owningBlock;

    bool updatedName = true;
    while (updatedName) {
        updatedName = false;

        for (int i = term->index-1; i >= 0; i--) {
            Term* other = block->get(i);
            if (other == NULL) continue;

            // If another term shares the same base, then make sure our ordinal is
            // higher. This turns some O(n) cases into O(1)
            if (string_eq(&other->uniqueName.base, &name.base)
                    && (other->uniqueName.ordinal >= name.ordinal)) {
                name.ordinal = other->uniqueName.ordinal + 1;
                updatedName = true;

            // If this name is already used, then just try the next ordinal. This
            // case results in more blind searching, but it's necessary to handle
            // the situation where a generated name is already taken.
            } else if (string_eq(&other->uniqueName.name, &name.name)) {
                name.ordinal++;
                updatedName = true;
            }

            if (updatedName) {
                char ordinalBuf[30];
                sprintf(ordinalBuf, "%d", name.ordinal);
                copy(&name.base, &name.name);
                string_append(&name.name, "_");
                string_append(&name.name, ordinalBuf);
                break;
            }
        }
    }
}

caValue* get_unique_name(Term* term)
{
    return &term->uniqueName.name;
}

Term* find_from_unique_name(Block* block, const char* name)
{
    // O(n) search; this should be made more efficient.

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;
        if (string_eq(get_unique_name(term), name)) {
            return term;
        }
    }
    return NULL;
}

Type* find_type(World* world, const char* name)
{
    caTerm* term = find_name(world->root, name, -1, sym_LookupType);
    if (term == NULL)
        return NULL;
    return circa_type(circa_term_value(term));
}
Type* find_type_local(Block* block, const char* name)
{
    caTerm* term = find_name(block, name, -1, sym_LookupType);
    if (term == NULL)
        return NULL;
    return circa_type(circa_term_value(term));
}
Block* find_function(World* world, const char* name)
{
    caTerm* term = find_name(world->root, name, -1, sym_LookupFunction);
    if (term == NULL)
        return NULL;
    return function_contents(term);
}
Block* find_function_local(Block* block, const char* name)
{
    caTerm* term = find_name(block, name, -1, sym_LookupFunction);
    if (term == NULL)
        return NULL;
    return function_contents(term);
}
Block* find_module(World* world, const char* name)
{
    caTerm* term = find_name(world->root, name, -1, sym_LookupModule);
    if (term == NULL)
        return NULL;
    return nested_contents(term);
}

void qualified_name_get_first_section(caValue* name, caValue* prefixResult)
{
    int len = string_length(name);
    for (int i=0; i < len; i++) {
        if (string_get(name, i) == ':') {
            string_slice(name, 0, i, prefixResult);
            return;
        }
    }
    set_null(prefixResult);
}
void qualified_name_get_remainder_after_first_section(caValue* name, caValue* suffixResult)
{
    int len = string_length(name);
    for (int i=0; i < len; i++) {
        if (string_get(name, i) == ':') {
            string_slice(name, i + 1, -1, suffixResult);
            return;
        }
    }
    set_null(suffixResult);
}

#if 0
Symbol existing_name_from_string(const char* str)
{
    INCREMENT_STAT(InternedNameLookup);

    std::map<std::string,Name>::const_iterator it;
    it = g_stringToSymbol.find(str);
    if (it != g_stringToSymbol.end())
        return it->second;

    return 0;
}

Symbol existing_name_from_string(const char* str, int len)
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
Symbol sym_from_string(const char* str)
{
    // Empty string is sym_None
    if (str[0] == 0)
        return sym_None;

    // Check if name is already registered
    Symbol existing = existing_name_from_string(str);
    if (existing != sym_None)
        return existing;

    // Not yet registered; add it to the list.
    INCREMENT_STAT(InternedNameCreate);

    Symbol index = g_nextFreeNameIndex++;
    g_runtimeNames[index].str = strdup(str);
    g_runtimeNames[index].namespaceFirst = 0;
    g_runtimeNames[index].namespaceRightRemainder = 0;
    Symbol name = index + c_FirstRuntimeName;
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

            g_runtimeNames[index].namespaceFirst = sym_from_string(tempstr);
            g_runtimeNames[index].namespaceRightRemainder = sym_from_string(str + i + 1);
            free(tempstr);
            break;
        }
    }

    return name;
}
Symbol sym_from_string(std::string const& str)
{
    return sym_from_string(str.c_str());
}
Symbol sym_from_string(caValue* str)
{
    return sym_from_string(as_cstring(str));
}
void sym_dealloc_global_data()
{
    for (int i=0; i < g_nextFreeNameIndex; i++)
        free(g_runtimeNames[i].str);
    g_nextFreeNameIndex = 0;
    g_stringToSymbol.clear();
}
#endif

} // namespace circa

#if 0
extern "C" caSymbol circa_to_name(const char* str)
{
    return circa::sym_from_string(str);
}
extern "C" const char* circa_name_to_string(caSymbol name)
{
    return circa::name_to_string(name);
}
#endif
