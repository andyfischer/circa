// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

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
    }

    _terms.clear();
}

Branch& Branch::operator=(Branch const& b)
{
    // Const hack because not everything is const-correct
    Branch& b_unconst = const_cast<Branch&>(b);

    clear();

    duplicate_branch(b_unconst, *this);

    return *this;
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
    return parser::evaluate_statement(*this, statement);
}

Term*
Branch::compile(std::string const& statement)
{
    return parser::compile_statement(*this, statement);
}

Branch& Branch::startBranch(std::string const& name)
{
    Term* result = create_value(this, BRANCH_TYPE, name);
    as_branch(result).outerScope = this;
    return as_branch(result);
}

void Branch::copy(Term* source, Term* dest)
{
    as_branch(dest).clear();
    duplicate_branch(as_branch(source), as_branch(dest));
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

void duplicate_branch_nested(ReferenceMap& newTermMap, Branch& source, Branch& dest)
{
    // Duplicate every term
    for (int index=0; index < source.numTerms(); index++) {
        Term* source_term = source.get(index);

        Term* dest_term = create_duplicate(&dest, source_term, false);

        newTermMap[source_term] = dest_term;

        if (dest_term->state != NULL && dest_term->state->type == BRANCH_TYPE) {
            duplicate_branch_nested(newTermMap, as_branch(source_term->state),
                    as_branch(dest_term->state));
        }

        // Copy names
        if (source_term->name != "")
            dest.bindName(dest_term, source_term->name);
    }
}

void duplicate_branch(Branch& source, Branch& dest)
{
    ReferenceMap newTermMap;

    duplicate_branch_nested(newTermMap, source, dest);

    // Remap pointers
    for (int index=0; index < dest.numTerms(); index++) {
        Term* term = dest.get(index);
        remap_pointers(term, newTermMap);
    }
}


void evaluate_file(Branch& branch, std::string const& filename)
{
    std::string fileContents = read_text_file(filename);

    parser::compile(branch, parser::statement_list, fileContents);

    string_value(branch, filename, get_name_for_attribute("source-file"));
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
