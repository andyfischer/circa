// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

Branch::Branch()
  : owningTerm(NULL)
{
}

Branch::~Branch()
{
    names.clear();
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

void Branch::remove(Term* term)
{
    // remove name binding if necessary
    if (term != NULL && (term->name != "") && (names[term->name] == term))
        names.remove(term->name);

    _terms.remove(term);
    term->owningBranch = NULL;
}

void Branch::remove(std::string const& name)
{
    if (!names.contains(name))
        return;

    Term* term = names[name];

    names.remove(name);
    _terms.remove(term);
    term->owningBranch = NULL;
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
    if (term->name != "" && term->name != name) {
        throw std::runtime_error("term already has name: "+term->name);
    }

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

Term*
Branch::eval(std::string const& statement)
{
    return parser::evaluate_statement(*this, statement);
}

std::string Branch::toString()
{
    std::stringstream out;
    out << "{ ";
    for (int i=0; i < length(); i++) {
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
    return parser::compile(this, parser::statement_list, statement);
}

void* Branch::alloc(Term* typeTerm)
{
    Branch* branch = new Branch();

    // create a slot for each field
    Type& type = as_type(typeTerm);
    int numFields = type.numFields();

    for (int f=0; f < numFields; f++)
        create_value(branch, type.prototype[f]->type, type.prototype[f]->name);

    return branch;
}

void Branch::dealloc(void* data)
{
    delete (Branch*) data;
}

void Branch::assign(Term* sourceTerm, Term* destTerm)
{
    Branch& source = as_branch(sourceTerm);
    Branch& dest = as_branch(destTerm);

    // Assign terms as necessary
    int lengthToAssign = std::min(source.length(), dest.length());

    for (int i=0; i < lengthToAssign; i++) {
        assign_value(source[i], dest[i]);
    }

    // Add terms if necessary
    for (int i=dest.length(); i < source.length(); i++) {
        Term* t = create_duplicate(&dest, source[i]);
        if (source[i]->name != "")
            dest.bindName(t, source[i]->name);
    }

    // Remove terms if necessary
    for (int i=source.length(); i < dest.length(); i++) {
        dest[i] = NULL;
    }

    dest.removeNulls();
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
    alloc_value(term);
    return *((Branch*) term->value);
}

std::string get_name_for_attribute(std::string attribute)
{
    return "#attr:" + attribute;
}

Term* get_branch_attribute(Branch& branch, std::string const& attr)
{
    std::string name = get_name_for_attribute(attr);
    if (branch.contains(name))
        return branch[name];
    else
        return NULL;
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

        Term* dest_term = create_duplicate(&dest, source_term, false);

        newTermMap[source_term] = dest_term;

        // if output is a branch, duplicate it
        if (is_branch(source_term) && is_value_alloced(source_term)) {
            as_branch(dest_term).clear();
            duplicate_branch_nested(newTermMap, as_branch(source_term), as_branch(dest_term));
        }

        // Copy names
        if (source_term->name != "")
            dest.bindName(dest_term, source_term->name);
    }
}

Branch& create_branch(Branch* owner, std::string const& name)
{
    Term* term = apply(owner, BRANCH_FUNC, RefList(), name);
    alloc_value(term);
    return as_branch(term);
}

void duplicate_branch(Branch& source, Branch& dest)
{
    ReferenceMap newTermMap;

    duplicate_branch_nested(newTermMap, source, dest);

    // Remap pointers
    for (int i=0; i < dest.length(); i++)
        remap_pointers(dest[i], newTermMap);

    dest.names = source.names;
    dest.names.remapPointers(newTermMap);
}

void parse_script(Branch& branch, std::string const& filename)
{
    std::string fileContents = read_text_file(filename);

    parser::compile(&branch, parser::statement_list, fileContents);

    // record the filename
    string_value(&branch, filename, get_name_for_attribute("source-file"));
}

void evaluate_script(Branch& branch, std::string const& filename)
{
    parse_script(branch, filename);
    evaluate_branch(branch);
}

Term* find_named(Branch* branch, std::string const& name)
{
    if (branch != NULL) {
        if (branch->contains(name))
            return branch->get(name);

        return find_named(get_outer_scope(*branch), name);
    }

    return get_global(name);
}

bool reload_branch_from_file(Branch& branch, std::ostream &errors)
{
    std::string filename = as_string(branch[get_name_for_attribute("source-file")]);

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
    std::string filename = as_string(branch[get_name_for_attribute("source-file")]);
    write_text_file(filename, get_branch_source(branch) + "\n");
}

} // namespace circa
