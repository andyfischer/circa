// Copyright 2008 Andrew Fischer

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
    for (unsigned int i = 0; i < _terms.size(); i++)
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

    // Find any terms that are using our terms, and tell them to stop.
    {
        PointerIterator *it = start_branch_pointer_iterator(this);

        while (!it->finished()) {
            Term* term = it->current();
            assert(term != NULL);

            assert_good_pointer(term);

            for (int userIndex=0; userIndex < (int) term->users.count(); userIndex++) {
                Term* user = term->users[userIndex];
                if (user == NULL)
                    continue;
                assert_good_pointer(user);
                if (user->owningBranch == this) {
                    term->users[userIndex] = NULL;
                }
            }
            term->users.removeNulls();
            it->advance();
        }
        delete it;
    }

    // Create a map where all our terms go to NULL
    /*
    ReferenceMap deleteMap;
    
    std::vector<Term*>::iterator it;
    for (it = _terms.begin(); it != _terms.end(); ++it) {
        if (*it != NULL)
            deleteMap[*it] = NULL;
    }

    this->remapPointers(deleteMap);
    */

    // delete everybody
    for (unsigned int i = 0; i < _terms.size(); i++)
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
    _terms.push_back(term);
}

void Branch::bindName(Term* term, std::string name)
{
    names.bind(term, name);

#if 0
    // enable this code when subroutine inputs can work without having
    // multiple name bindings.
    if (term->name != "") {
        throw std::runtime_error(std::string("term already has name: ")+term->name);
    }
#endif

    term->name = name;
}

Term* Branch::findNamed(std::string const& name) const
{
    if (containsName(name))
        return getNamed(name);

    if (outerScope != NULL) {
        return outerScope->findNamed(name);
    }

    return get_global(name);
}

void Branch::remapPointers(ReferenceMap const& map)
{
    names.remapPointers(map);

    std::vector<Term*>::iterator it;
    for (it = _terms.begin(); it != _terms.end(); ++it) {
        if (*it != NULL)
            remap_pointers(*it, map);
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

    std::vector<Term*>::iterator it;
    for (it = _terms.begin(); it != _terms.end(); ++it) {
        if (*it == NULL)
            continue;

        // Type& type = as_type((*it)->type);

        visit_pointers(*it, myVisitor);
    }
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

        Term* dest_term = create_term(&dest, source_term->function, source_term->inputs);
        newTermMap[source_term] = dest_term;

        duplicate_value(source_term, dest_term);
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

void migrate_branch(Branch& original, Branch& replacement)
{
    // This function is a work in progress

    // For now, just look for named terms in 'original', and replace
    // values with values from 'replacement'
    for (int termIndex=0; termIndex < original.numTerms(); termIndex++) {
        Term* term = original[termIndex];
        if (term->name == "")
            continue;

        if (!replacement.containsName(term->name))
            // todo: print a warning here?
            continue;

        // Don't migrate terms with a 'should-migrate' set to false
        // This might be a temporary measure.
        if (term->hasProperty("should-migrate")
                && !as_bool(term->property("should-migrate")))
            continue;

        Term* replaceTerm = replacement[term->name];

        // Special behavior for branches
        if (term->state != NULL && term->state->type == BRANCH_TYPE) {
            // branch migration
            migrate_branch(as_branch(term->state), as_branch(replaceTerm->state));
        } else if (is_value(term) && term->type == FUNCTION_TYPE) {
            // subroutine migration
            migrate_branch(get_subroutine_branch(term), get_subroutine_branch(replaceTerm));
        } else if (is_value(term)) {
            // value migration
            assign_value(replaceTerm, term);
        }
    }
}

void evaluate_file(Branch& branch, std::string const& filename)
{
    std::string fileContents = read_text_file(filename);

    token_stream::TokenStream tokens(fileContents);
    ast::StatementList *statementList = parser::statementList(tokens);

    CompilationContext context;
    context.pushScope(&branch, NULL);
    statementList->createTerms(context);
    context.popScope();

    delete statementList;

    string_value(branch, filename, get_name_for_attribute("source-file"));
}

void reload_branch_from_file(Branch& branch)
{
    std::string filename = as_string(branch[get_name_for_attribute("source-file")]);

    Branch replacement;

    evaluate_file(replacement, filename);

    migrate_branch(branch, replacement);
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
    if (branch == NULL)
        return get_global(name);
    else
        return branch->findNamed(name);
}

} // namespace circa
