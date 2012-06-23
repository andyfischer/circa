// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/file.h"

#include "building.h"
#include "code_iterators.h"
#include "evaluation.h"
#include "file_utils.h"
#include "list.h"
#include "kernel.h"
#include "modules.h"
#include "names.h"
#include "static_checking.h"
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"

namespace circa {

List g_moduleSearchPaths;

List* modules_get_search_paths()
{
    return &g_moduleSearchPaths;
}

void modules_add_search_path(const char* str)
{
    if (strcmp(str, "") == 0)
        internal_error("blank path in modules_add_search_path");

    set_string(g_moduleSearchPaths.append(), str);
}

Branch* find_loaded_module(const char* name)
{
    for (BranchIteratorFlat it(kernel()); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (term->function == FUNCS.imported_file && term->name == name)
            return nested_contents(term);
    }
    return NULL;
}

Branch* load_module_from_file(const char* module_name, const char* filename)
{
    Term* import = apply(kernel(), FUNCS.imported_file, TermList(), module_name);
    load_script(nested_contents(import), filename);
    return nested_contents(import);
}

static bool find_module_file(const char* module_name, caValue* filenameOut)
{
    Value module;
    set_string(&module, module_name);

    int count = list_length(&g_moduleSearchPaths);
    for (int i=0; i < count; i++) {

        // For each search path we'll check two places.

        // Look under searchPath/moduleName.ca
        Value searchPath;
        copy(g_moduleSearchPaths[i], &searchPath);
        circa_join_path(&searchPath, &module);
        string_append(&searchPath, ".ca");

        if (circa_file_exists(as_cstring(&searchPath))) {
            swap(&searchPath, filenameOut);
            return true;
        }

        // Look under searchPath/moduleName/moduleName.ca
        copy(g_moduleSearchPaths[i], &searchPath);

        circa_join_path(&searchPath, &module);
        circa_join_path(&searchPath, &module);
        string_append(&searchPath, ".ca");

        if (circa_file_exists(as_cstring(&searchPath))) {
            swap(&searchPath, filenameOut);
            return true;
        }
    }
    return false;
}

Branch* load_module(const char* module_name, Term* loadCall)
{
    Branch* existing = find_loaded_module(module_name);
    if (existing != NULL)
        return existing;
    
    Value filename;
    bool found = find_module_file(module_name, &filename);

    if (!found)
        return NULL;

    Term* import = load_module_from_file(module_name, as_cstring(&filename))->owningTerm;

    // If a loadCall is provided, possibly move the new import to be before the loadCall.
    if (loadCall != NULL) {
        Term* callersModule = find_parent_term_in_branch(loadCall, import->owningBranch);

        if (callersModule != NULL && (import->index > callersModule->index))
            move_before(import, callersModule);
    }

    // If the module has static errors, print them now.
    print_static_errors_formatted(nested_contents(import));

    return nested_contents(import);
}

Branch* find_module_from_filename(const char* filename)
{
    // O(n) search for a module with this filename. Could stand to be more efficient.
    for (int i=0; i < kernel()->length(); i++) {
        Term* term = kernel()->get(i);
        if (term->nestedContents == NULL)
            continue;

        caValue* branchFilename = branch_get_source_filename(nested_contents(term));
        if (branchFilename == NULL)
            continue;

        if (string_eq(branchFilename, filename))
            return nested_contents(term);
    }

    return NULL;
}

void get_relative_name(Term* term, Branch* relativeTo, caValue* nameOutput)
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

Term* find_from_relative_name(caValue* name, Branch* relativeTo)
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

// Returns the corresponding term inside newBranch, if found.
// Returns 'term' if the translation does not apply (term is not found inside
// oldBranch).
// Returns NULL if the translation does apply, but a corresponding term cannot be found.
Term* translate_term_across_branches(Term* term, Branch* oldBranch, Branch* newBranch)
{
    if (!term_is_child_of_branch(term, oldBranch))
        return term;

    Value relativeName;
    get_relative_name(term, oldBranch, &relativeName);
    return find_from_relative_name(&relativeName, newBranch);
}

void update_all_code_references(Branch* target, Branch* oldBranch, Branch* newBranch)
{
    ca_assert(target != oldBranch);
    ca_assert(target != newBranch);

    // Store a cache of lookups that we've made in this call.
    TermMap cache;

    for (BranchIterator it(target); it.unfinished(); it.advance()) {

        Term* term = *it;

        // Iterate through each "dependency", which includes the function & inputs.
        for (int i=0; i < term->numDependencies(); i++) {
            Term* ref = term->dependency(i);
            Term* newRef = NULL;

            if (cache.contains(ref)) {
                newRef = cache[ref];
            } else {

                // Lookup and save result in cache
                newRef = translate_term_across_branches(ref, oldBranch, newBranch);
                cache[ref] = newRef;
            }

            // Possibly rebind
            if (newRef != ref)
                term->setDependency(i, newRef);
        }
    }
}

} // namespace circa

using namespace circa;

// Public API
extern "C" {

void circa_run_module(caStack* stack, const char* moduleName)
{
    circa::Branch* branch = nested_contents(get_global(moduleName));

    evaluate_branch((circa::Stack*) stack, branch);
}

void circa_add_module_search_path(caWorld* world, const char* path)
{
    modules_add_search_path(path);
}

caBranch* circa_load_module_from_file(caWorld*, const char* module_name, const char* filename)
{
    return (caBranch*) load_module_from_file(module_name, filename);
}

} // extern "C"
