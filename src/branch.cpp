// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

Branch::~Branch()
{
    names.clear();
    _terms.clear();
}

int Branch::findIndex(Term* term)
{
    for (int i=0; i < length(); i++) 
        if (get(i) == term)
            return i;
    return -1;
}

int Branch::findIndex(std::string const& name)
{
    for (int i=0; i < length(); i++) {
        if (get(i) == NULL)
            continue;
        if (get(i)->name == name)
            return i;
    }
    return -1;
}

void Branch::append(Term* term)
{
    _terms.append(term);
    if (term != NULL) {
        assert(term->owningBranch == NULL);
        term->owningBranch = this;
    }
}

void Branch::insert(int index, Term* term)
{
    _terms.insert(index, term);
    if (term != NULL) {
        assert(term->owningBranch == NULL);
        term->owningBranch = this;
    }
}

void Branch::moveToEnd(Term* term)
{
    assert(term->owningBranch == this);
    int index = _terms.findIndex(term);
    assert(index >= 0);
    _terms.append(term); // do this first so that the term doesn't lose references
    _terms[index] = NULL;
}

void Branch::remove(Term* term)
{
    remove(findIndex(term));
}

void Branch::remove(std::string const& name)
{
    if (!names.contains(name))
        return;

    Term* term = names[name];

    remove(findIndex(term));
}

void Branch::remove(int index)
{
    if (index >= _terms.length())
        return;

    Term* term = _terms[index];

    // remove name binding if necessary
    if (term != NULL && (term->name != "") && (names[term->name] == term))
        names.remove(term->name);

    _terms.remove(index);
    term->owningBranch = NULL;
}

void Branch::removeNulls()
{
    _terms.removeNulls();
}

void Branch::shorten(int newLength)
{
    for (int i=newLength; i < length(); i++)
        get(i) = NULL;

    removeNulls();
}

Term* Branch::findFirstBinding(std::string const& name) const
{
    for (int i = 0; i < _terms.length(); i++) {
        if (_terms[i] == NULL)
            continue;
        if (_terms[i]->name == name)
            return _terms[i];
    }

    return NULL;
}

Term* Branch::findLastBinding(std::string const& name) const
{
    for (int i = _terms.length()-1; i >= 0; i--) {
        if (_terms[i] == NULL)
            continue;
        if (_terms[i]->name == name)
            return _terms[i];
    }

    return NULL;
}

void Branch::bindName(Term* term, std::string name)
{
    if (term->name != "" && term->name != name)
        throw std::runtime_error("term already has name: "+term->name);

    names.bind(term, name);
    term->name = name;
}

void Branch::remapPointers(ReferenceMap const& map)
{
    names.remapPointers(map);

    for (int i = 0; i < _terms.length(); i++) {
        Term* term = _terms[i];
        if (term != NULL)
            remap_pointers(term, map);
    }
}

void
Branch::clear()
{
    for (int i=0; i < _terms.length(); i++)
        _terms[i]->owningBranch = NULL;

    _terms.clear();
    names.clear();
}

std::string Branch::toString()
{
    std::stringstream out;
    out << "[";
    for (int i=0; i < length(); i++) {
        Term* term = get(i);
        if (i > 0) out << ", ";
        if (term->name != "")
            out << term->name << ": ";
        out << term->toString();
    }
    out << "]";
    return out.str();
}

Term*
Branch::compile(std::string const& code)
{
    return parser::compile(this, parser::statement_list, code);
}

Term*
Branch::eval(std::string const& code)
{
    return parser::evaluate(*this, parser::statement_list, code);
}

namespace branch_t {
    void alloc(Term* typeTerm, Term* term)
    {
        Branch* branch = new Branch();

        Branch& prototype = type_t::get_prototype(typeTerm);

        duplicate_branch(prototype, *branch);

        term->value = branch;
    }

    void dealloc(Term* type, Term* term)
    {
        delete (Branch*) term->value;
    }

    void assign(Term* sourceTerm, Term* destTerm)
    {
        Branch& source = as_branch(sourceTerm);
        Branch& dest = as_branch(destTerm);
        bool strictTypes = destTerm->type != BRANCH_TYPE && destTerm->type != LIST_TYPE;

        // Assign terms as necessary
        int lengthToAssign = std::min(source.length(), dest.length());

        for (int i=0; i < lengthToAssign; i++) {
            // Change type if necessary
            if (!strictTypes && (source[i]->type != dest[i]->type))
                change_type(dest[i], source[i]->type);
            assign_value(source[i], dest[i]);
        }

        // Add terms if necessary
        for (int i=dest.length(); i < source.length(); i++) {
            Term* t = create_duplicate(dest, source[i]);
            if (source[i]->name != "")
                dest.bindName(t, source[i]->name);
        }

        // Remove terms if necessary
        for (int i=source.length(); i < dest.length(); i++) {
            dest[i] = NULL;
        }

        dest.removeNulls();
    }

    void hosted_remap_pointers(Term* caller, ReferenceMap const& map)
    {
        as_branch(caller).remapPointers(map);
    }

    bool equals(Term* lhsTerm, Term* rhsTerm)
    {
        Branch& lhs = as_branch(lhsTerm);
        Branch& rhs = as_branch(rhsTerm);

        if (lhs.length() != rhs.length())
            return false;

        for (int i=0; i < lhs.length(); i++) {
            if (!circa::equals(lhs[i], rhs[i]))
                return false;
        }

        return true;
    }
}

bool is_branch(Term* term)
{
    return (term->type != NULL) &&
        (type_t::get_alloc_func(term->type) == branch_t::alloc);
}

Branch& as_branch(Term* term)
{
    assert(is_branch(term));
    alloc_value(term);
    return *((Branch*) term->value);
}

bool is_namespace(Term* term)
{
    return is_branch(term) && term->type == NAMESPACE_TYPE;
}

std::string get_branch_source_filename(Branch& branch)
{
    Term* attr = branch["#attr:source-file"];
    if (attr == NULL)
        return "";
    else
        return as_string(attr);
}

Branch* get_outer_scope(Branch& branch)
{
    if (branch.owningTerm == NULL)
        return NULL;
    return branch.owningTerm->owningBranch;
}

Term* find_term_by_id(Branch& branch, unsigned int id)
{
    for (BranchIterator it(branch); !it.finished(); it.advance()) {
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
    for (int index=0; index < source.length(); index++) {
        Term* source_term = source.get(index);

        Term* dest_term = create_duplicate(dest, source_term, source_term->name, false);

        newTermMap[source_term] = dest_term;

        // if output is a branch, duplicate it
        if (is_branch(source_term) && is_value_alloced(source_term)) {
            as_branch(dest_term).clear();
            duplicate_branch_nested(newTermMap, as_branch(source_term), as_branch(dest_term));
        }
    }
}

void duplicate_branch(Branch& source, Branch& dest)
{
    ReferenceMap newTermMap;

    duplicate_branch_nested(newTermMap, source, dest);

    Timer timer;

    // Remap pointers
    for (int i=0; i < dest.length(); i++)
        remap_pointers(dest[i], newTermMap);

    // Include/overwrite names
    dest.names.append(source.names);
    dest.names.remapPointers(newTermMap);
}

void parse_script(Branch& branch, std::string const& filename)
{
    // Record the filename
    create_string(branch, filename, "#attr:source-file");

    std::string fileContents = read_text_file(filename);

    parser::compile(&branch, parser::statement_list, fileContents);
}

void evaluate_script(Branch& branch, std::string const& filename)
{
    parse_script(branch, filename);
    evaluate_branch(branch);
}

bool reload_branch_from_file(Branch& branch, std::ostream &errors)
{
    std::string filename = get_branch_source_filename(branch);

    Branch replacement;
    replacement.owningTerm = branch.owningTerm;
    parse_script(replacement, filename);

    if (has_static_errors(replacement)) {
        print_static_errors_formatted(replacement, errors);
        return false;
    }

    migrate_stateful_values(branch, replacement);

    branch.clear();
    duplicate_branch(replacement, branch);
    return true;
}

void persist_branch_to_file(Branch& branch)
{
    std::string filename = get_branch_source_filename(branch);
    write_text_file(filename, get_branch_source(branch) + "\n");
}

std::string get_source_file_location(Branch& branch)
{
    // Search upwards until we find a branch that has source-file defined.
    Branch* branch_p = &branch;

    while (branch_p != NULL && get_branch_source_filename(*branch_p) == "") {
        if (branch_p->owningTerm == NULL)
            branch_p = NULL;
        else
            branch_p = branch_p->owningTerm->owningBranch;
    }

    if (branch_p == NULL)
        return "";

    return get_directory_for_filename(get_branch_source_filename(*branch_p));
}

} // namespace circa
