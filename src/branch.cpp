// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "debug.h"
#include "function.h"
#include "introspection.h"
#include "parser.h"
#include "pointer_iterator.h"
#include "runtime.h"
#include "ref_map.h"
#include "term.h"
#include "term_pointer_iterator.h"
#include "type.h"
#include "values.h"
#include "wrappers.h"

#include "branch_iterators.hpp"

namespace circa {

Branch::~Branch()
{
    // Dealloc_value on all non-types
    for (unsigned int i = 0; i < _terms.count(); i++)
    {
        Term *term = _terms[i];

        if (term == NULL)
            continue;

        assert_good_pointer(term);

        if (term->type != TYPE_TYPE)
            dealloc_value(term);

        if (term->state != NULL)
            dealloc_value(term->state);
    }

    // Delete everybody
    for (unsigned int i = 0; i < _terms.count(); i++)
    {
        Term *term = _terms[i];
        if (term == NULL)
            continue;

        assert_good_pointer(term);

        dealloc_value(term);
        term->owningBranch = NULL;
        delete_term(term);
    }
}

void Branch::append(Term* term)
{
    assert_good_pointer(term);
    assert(term->owningBranch == NULL);
    term->owningBranch = this;
    _terms.append(term);
}

void Branch::removeTerm(std::string const& name)
{
    if (!names.contains(name))
        return;

    Term* term = names[name];

    names.remove(name);
    _terms.remove(term);
    // TODO: delete this term
}

Term* Branch::findFirstBinding(std::string const& name) const
{
    for (unsigned int i = 0; i < _terms.count(); i++) {
        if (_terms[i] == NULL)
            continue;
        if (_terms[i]->name == name)
            return _terms[i];
    }

    return NULL;
}

void Branch::bindName(Term* term, std::string name)
{
    names.bind(term, name);

    if (term->name != "") {
        throw std::runtime_error(std::string("term already has name: ")+term->name);
    }

    term->name = name;
}

void Branch::remapPointers(ReferenceMap const& map)
{
    names.remapPointers(map);

    for (unsigned int i = 0; i < _terms.count(); i++) {
        Term* term = _terms[i];
        if (term != NULL)
            remap_pointers(term, map);
    }
}

void Branch::visitPointers(PointerVisitor& visitor)
{
    struct VisitPointerIfOutsideBranch : PointerVisitor
    {
        Branch *branch;
        PointerVisitor &visitor;

        VisitPointerIfOutsideBranch(Branch *_branch, PointerVisitor &_visitor)
            : branch(_branch), visitor(_visitor) {}

        virtual void visitPointer(Term* term)
        {
            if (term == NULL)
                return;
            if (term->owningBranch != branch)
                visitor.visitPointer(term);
        }
    };

    VisitPointerIfOutsideBranch myVisitor(this, visitor);

    for (unsigned int i = 0; i < _terms.count(); i++) {
        Term* term = _terms[i];
        if (term == NULL)
            continue;

        visit_pointers(term, myVisitor);
    }
}

void Branch::_replaceTermObject(Term* existing, Term* replacement)
{
    int existingIndex = _terms.findIndex(existing);

    assert(existingIndex >= 0);

    _terms[existingIndex] = replacement;

    ReferenceMap map;
    map[existing] = replacement;

    remapPointers(map);
}

void
Branch::clear()
{
    _terms.clear();
    names.clear();
}

void Branch::eval()
{
    evaluate_branch(*this);
}

Term*
Branch::eval(std::string const& statement)
{
    return eval_statement(this, statement);
}

Term*
Branch::compile(std::string const& statement)
{
    return compile_statement(this, statement);
}

Branch& Branch::startBranch(std::string const& name)
{
    Term* result = create_value(this, BRANCH_TYPE, name);
    as_branch(result).outerScope = this;
    return as_branch(result);
}

void
Branch::hosted_remap_pointers(Term* caller, ReferenceMap const& map)
{
    as_branch(caller).remapPointers(map);
}

void
Branch::hosted_visit_pointers(Term* caller, PointerVisitor& visitor)
{
    as_branch(caller).visitPointers(visitor);
}

Branch& as_branch(Term* term)
{
    assert_type(term, BRANCH_TYPE);
    assert(term->value != NULL);
    return *((Branch*) term->value);
}

std::string get_name_for_attribute(std::string attribute)
{
    return "#attr:" + attribute;
}

void duplicate_branch(Branch& source, Branch& dest)
{
    ReferenceMap newTermMap;

    // Duplicate every term
    for (int index=0; index < source.numTerms(); index++) {
        Term* source_term = source.get(index);

        Term* dest_term = create_duplicate(&dest, source_term);

        newTermMap[source_term] = dest_term;
    }

    // Remap terms
    for (int index=0; index < dest.numTerms(); index++) {
        Term* term = dest.get(index);
        remap_pointers(term, newTermMap);
    }

    // Copy names
    TermNamespace::StringToTermMap::iterator it;
    for (it = source.names.begin(); it != source.names.end(); ++it) {
        std::string name = it->first;
        Term* original_term = it->second;
        dest.bindName(newTermMap.getRemapped(original_term), name);
    }
}

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
            assign_value(source, dest);
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

void evaluate_file(Branch& branch, std::string const& filename)
{
    std::string fileContents = read_text_file(filename);

    TokenStream tokens(fileContents);
    ast::StatementList *statementList = parser::statementList(tokens);

    statementList->createTerms(branch);

    delete statementList;

    remove_compilation_attrs(branch);

    string_value(branch, filename, get_name_for_attribute("source-file"));
}

void reload_branch_from_file(Branch& branch)
{
    std::string filename = as_string(branch[get_name_for_attribute("source-file")]);

    Branch replacement;

    evaluate_file(replacement, filename);

    migrate_branch(replacement, branch);
}


PointerIterator*
Branch::start_pointer_iterator(Term* term)
{
    return new BranchExternalPointerIterator((Branch*) term->value);
}

PointerIterator* start_branch_iterator(Branch* branch)
{
    return new BranchIterator(branch);
}

PointerIterator* start_branch_pointer_iterator(Branch* branch)
{
    return new BranchExternalPointerIterator(branch);
}

PointerIterator* start_branch_control_flow_iterator(Branch* branch)
{
    return new BranchControlFlowIterator(branch);
}

Term* find_named(Branch* branch, std::string const& name)
{
    if (branch != NULL) {
        if (branch->containsName(name))
            return branch->getNamed(name);

        return find_named(branch->outerScope, name);
    }

    return get_global(name);
}

} // namespace circa
