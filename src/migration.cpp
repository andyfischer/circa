// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

// Returns whether the term was migrated
bool migrate_term(Term* source, Term* dest)
{
    // Branch migration
    if (dest->state != NULL && dest->state->type == BRANCH_TYPE) {
        migrate_branch(as_branch(source->state),as_branch(dest->state));
        return true;
    } 
    
    // Subroutine migration
    if (is_value(dest) && dest->type == FUNCTION_TYPE) {
        migrate_branch(get_subroutine_branch(source),get_subroutine_branch(dest));
        return true;
    }

    // Value migration
    if (is_value(dest)) {

        // Don't overwrite value for state. But do migrate this term object
        if (!dest->isStateful()) {
            copy_value(source, dest);
        }

        return true;
    }

    return false;
}

void migrate_branch(Branch& replacement, Branch& target)
{
    // The goal:
    //
    // Modify 'target' so that it is roughly equivalent to 'replacement',
    // but with as much state preserved as possible.
    //
    // The algorithm:
    //
    // 1. Copy 'target' to a temporary branch 'original'
    // 2. Overwrite 'target' with the contents of 'replacement'
    // 3. For every term in 'original', look for a match inside 'replacement'.
    //    A 'match' is defined loosely on purpose, because we want to allow for
    //    any amount of cleverness. But at a minimum, if two terms have the same
    //    name, then they match.
    // 4. If a match is found, completely replace the relevant term inside
    //    'target' with the matching term from 'original'.
    // 5. Discard 'original'.
    //

    ReferenceList originalTerms = target._terms;
    TermNamespace originalNamespace = target.names;

    target.clear();
    duplicate_branch(replacement, target);

    // Go through every one of original's names, see if we can migrate them.
    TermNamespace::iterator it;
    for (it = originalNamespace.begin(); it != originalNamespace.end(); ++it)
    {
        std::string name = it->first;
        Term* originalTerm = it->second;

        // Skip if name isn't in replacement
        if (!target.names.contains(name)) {
            continue;
        }

        Term* targetTerm = target[name];

        // Skip if type doesn't match
        if (originalTerm->type != targetTerm->type) {
            continue;
        }

        bool migrated = migrate_term(targetTerm, originalTerm);

        if (migrated) {
            // Replace in list
            target._replaceTermObject(targetTerm, originalTerm);
            originalTerms.remove(originalTerm);
        }
    }
}

void reload_branch_from_file(Branch& branch)
{
    std::string filename = as_string(branch[get_name_for_attribute("source-file")]);

    Branch replacement;

    evaluate_file(replacement, filename);

    migrate_branch(replacement, branch);
}

} // namespace circa
