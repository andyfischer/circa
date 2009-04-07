// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

#include "branch_iterators.hpp"

namespace circa {

Branch::Branch()
  : outerScope(NULL)
{
}

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
    // Const cast because duplicate_branch is not const-correct
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

void Branch::remove(std::string const& name)
{
    if (!names.contains(name))
        return;

    Term* term = names[name];

    names.remove(name);
    _terms.remove(term);
}

void Branch::remove(int index)
{
    if (index >= (int) _terms.count())
        return;

    Term* term = _terms[index];

    if (term != NULL && (term->name != "") && (names[term->name] == term))
        names.remove(term->name);

    _terms.remove(index);
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

    if (term->name != "" && term->name != name)
        throw std::runtime_error("term already has name: "+term->name);

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

void
Branch::clear()
{
    for (unsigned int i=0; i < _terms.count(); i++)
        _terms[i]->owningBranch = NULL;

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

std::string Branch::toString()
{
    std::stringstream out;
    out << "{ ";
    for (int i=0; i < numTerms(); i++) {
        Term* term = get(i);
        if (i > 0) out << ", ";
        if (term->name != "")
            out << term->name << ": ";
        out << term->toString();
    }
    out << " }";
    return out.str();
}

Term*
Branch::compile(std::string const& statement)
{
    return parser::compile_statement(*this, statement);
}

void* Branch::alloc(Term* typeTerm)
{
    Branch* branch = new Branch();

    // create a slot for each field
    Type& type = as_type(typeTerm);
    int numFields = (int) type.fields.size();

    for (int f=0; f < numFields; f++)
        create_value(branch, type.fields[f].type, type.fields[f].name);

    return branch;
}

void Branch::dealloc(void* data)
{
    delete (Branch*) data;
}

void Branch::assign(Term* source, Term* dest)
{
    as_branch(dest).clear();
    duplicate_branch(as_branch(source), as_branch(dest));
}

void
Branch::hosted_remap_pointers(Term* caller, ReferenceMap const& map)
{
    as_branch(caller).remapPointers(map);
}

bool is_branch(Term* term)
{
    return as_type(term->type).alloc == Branch::alloc;
}

Branch& as_branch(Term* term)
{
    if (!is_branch(term))
        assert_type(term, BRANCH_TYPE);
    assert(term->value != NULL);
    return *((Branch*) term->value);
}

std::string get_name_for_attribute(std::string attribute)
{
    return "#attr:" + attribute;
}

Term* find_term_by_id(Branch& branch, unsigned int id)
{
    for (CodeIterator it(&branch); !it.finished(); it.advance()) {
        if (*it == NULL)
            continue;

        if (it->globalID == id)
            return *it;
    }

    return NULL;
}

void duplicate_branch_nested(ReferenceMap& newTermMap, Branch& source, Branch& dest)
{
    // Duplicate every term
    for (int index=0; index < source.numTerms(); index++) {
        Term* source_term = source.get(index);

        Term* dest_term = create_duplicate(&dest, source_term, false);

        newTermMap[source_term] = dest_term;

        // if output or state is a branch, duplicate them
        if (is_branch(source_term)) {
            as_branch(dest_term).clear();
            duplicate_branch_nested(newTermMap, as_branch(source_term), as_branch(dest_term));
        }

        if (source_term->state != NULL && is_branch(source_term->state)) {
            as_branch(dest_term->state).clear();
            duplicate_branch_nested(newTermMap, as_branch(source_term->state), as_branch(dest_term->state));
        }

        // Special case for Function. These guys have a branch inside their value.
        if (source_term->type == FUNCTION_TYPE)
            duplicate_branch_nested(newTermMap, *get_inner_branch(source_term),
                    *get_inner_branch(dest_term));

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
    for (int i=0; i < dest.numTerms(); i++)
        remap_pointers(dest[i], newTermMap);

    dest.names = source.names;
    dest.names.remapPointers(newTermMap);
}

void evaluate_file(Branch& branch, std::string const& filename)
{
    std::string fileContents = read_text_file(filename);

    parser::compile(&branch, parser::statement_list, fileContents);

    string_value(&branch, filename, get_name_for_attribute("source-file"));
}

ReferenceIterator* start_branch_reference_iterator(Branch* branch)
{
    return new BranchExternalReferenceIterator(branch);
}

Term* find_named(Branch* branch, std::string const& name)
{
    if (branch != NULL) {
        if (branch->contains(name))
            return branch->getNamed(name);

        return find_named(branch->outerScope, name);
    }

    return get_global(name);
}

bool branch_has_outer_scope(Branch& branch)
{
    return branch.contains(get_name_for_attribute("outer_scope"));
}

Term*& branch_get_outer_scope(Branch& branch)
{
    if (!branch_has_outer_scope(branch))
        return deref(create_value(&branch, REF_TYPE,
                                    get_name_for_attribute("outer_scope")));

    return deref(branch[get_name_for_attribute("outer_scope")]);
}

void migrate_terms(Branch& source, Branch& dest)
{
    // Iterate over every term in 'source' and compare each with
    // every term in 'dest'. Lots of room for optimization here.
    for (int sourceIndex=0; sourceIndex < source.numTerms(); sourceIndex++) {
        for (int destIndex=0; destIndex < dest.numTerms(); destIndex++) {
            Term* sourceTerm = source[sourceIndex];
            Term* destTerm = dest[destIndex];

            if (sourceTerm == NULL) continue;
            if (destTerm == NULL) continue;

            if (sourceTerm->name == "") continue;
            if (sourceTerm->name != destTerm->name) continue;
            if (sourceTerm->type != destTerm->type) continue;
            if (sourceTerm->function != destTerm->function) continue;

            // At this point, they match

            // Branch migration
            if (has_inner_branch(sourceTerm)) {
                migrate_terms(*get_inner_branch(sourceTerm),*get_inner_branch(destTerm));
            } 
            
            // Stateful value migration
            else if (is_stateful(sourceTerm) && is_stateful(destTerm)) {
                assign_value(sourceTerm, destTerm);
            }
        }
    }
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

    Branch original_target;
    duplicate_branch(target, original_target);

    target.clear();
    duplicate_branch(replacement, target);

    migrate_terms(original_target, target);
}

void reload_branch_from_file(Branch& branch)
{
    std::string filename = as_string(branch[get_name_for_attribute("source-file")]);

    Branch replacement;

    evaluate_file(replacement, filename);

    migrate_branch(replacement, branch);
}

} // namespace circa
