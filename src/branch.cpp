// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "debug_valid_objects.h"
#include "importing_macros.h"

namespace circa {

static void assert_valid_branch(Branch const* obj)
{
    debug_assert_valid_object((void*) obj, BRANCH_OBJECT);
}

Branch::Branch()
  : owningTerm(NULL),
    _refCount(0),
    outputIndex(0),
    inuse(false)
{
    debug_register_valid_object((void*) this, BRANCH_OBJECT);
}

Branch::~Branch()
{
    names.clear();

    // Turn all our terms into orphans
    for (int i=0; i < _terms.length(); i++) {
        Term* term = _terms[i];
        if (term == NULL) continue;
        clear_all_users(term);
        term->owningBranch = NULL;
    }

    _terms.clear();
    debug_unregister_valid_object(this);
}

int Branch::length() const
{
    assert_valid_branch(this);
    return _terms.length();
}

bool Branch::contains(std::string const& name) const
{
    return get(name) != NULL;
}

Term* Branch::get(int index) const
{
    assert_valid_branch(this);
    if (index > length())
        throw std::runtime_error("index out of range");
    return _terms[index];
}

Term* Branch::last() const
{
    if (length() == 0) return NULL;
    else return _terms[length()-1];
}

int Branch::getIndex(Term* term) const
{
    ca_assert(term != NULL);
    ca_assert(term->owningBranch == this);
    assert_valid_term(term);

    //assert(term->index == debugFindIndex(term));

    return term->index;
}

int Branch::debugFindIndex(Term* term) const
{
    for (int i=0; i < length(); i++) 
        if (get(i) == term)
            return i;
    return -1;
}

int Branch::findIndex(std::string const& name) const
{
    for (int i=0; i < length(); i++) {
        if (get(i) == NULL)
            continue;
        if (get(i)->name == name)
            return i;
    }
    return -1;
}
int Branch::findIndex(const char* name) const
{
    for (int i=0; i < length(); i++) {
        if (get(i) == NULL)
            continue;
        if (get(i)->name == name)
            return i;
    }
    return -1;
}

void Branch::set(int index, Term* term)
{
    assert_valid_branch(this);
    ca_assert(index <= length());

    // No-op if this is the same term. Need to check this because otherwise
    // we decrement refcount on term.
    if (_terms[index] == term)
        return;

    setNull(index);
    _terms[index] = term;
    if (term != NULL) {
        assert_valid_term(term);
        ca_assert(term->owningBranch == NULL || term->owningBranch == this);
        term->owningBranch = this;
        term->index = index;
    }

    // TODO: update name bindings
}

void Branch::setNull(int index)
{
    assert_valid_branch(this);
    ca_assert(index <= length());
    Term* term = _terms[index];
    if (term != NULL) {
        // remove name binding if necessary
        if ((term->name != "") && (names[term->name] == term))
            names.remove(term->name);

        term->owningBranch = NULL;
        term->index = 0;
        _terms[index] = NULL;
    }
}

void Branch::append(Term* term)
{
    assert_valid_branch(this);
    _terms.append(term);
    if (term != NULL) {
        assert_valid_term(term);
        ca_assert(term->owningBranch == NULL);
        term->owningBranch = this;
        term->index = _terms.length()-1;
    }
}

Term* Branch::appendNew()
{
    assert_valid_branch(this);
    Term* term = alloc_term();
    ca_assert(term != NULL);
    _terms.append(term);
    term->owningBranch = this;
    term->index = _terms.length()-1;
    return term;
}

void Branch::insert(int index, Term* term)
{
    assert_valid_term(term);
    assert_valid_branch(this);
    ca_assert(index >= 0);
    ca_assert(index <= _terms.length());

    _terms.append(NULL);
    for (int i=_terms.length()-1; i > index; i--) {
        _terms[i] = _terms[i-1];
        _terms[i]->index = i;
    }
    _terms[index] = term;

    if (term != NULL) {
        ca_assert(term->owningBranch == NULL);
        term->owningBranch = this;
        term->index = index;
    }
}

void Branch::move(Term* term, int index)
{
    ca_assert(term->owningBranch == this);

    if (term->index == index)
        return;

    int dir = term->index < index ? 1 : -1;

    Ref ref = term;

    for (int i=term->index; i != index; i += dir) {
        _terms[i] = _terms[i+dir];
        _terms[i]->index = i;
    }
    _terms[index] = term;
    term->index = index;
}

void Branch::moveToEnd(Term* term)
{
    assert_valid_term(term);
    ca_assert(term != NULL);
    ca_assert(term->owningBranch == this);
    ca_assert(term->index >= 0);
    int index = getIndex(term);
    _terms.append(term); // do this first so that the term doesn't lose references
    _terms[index] = NULL;
    term->index = _terms.length()-1;
}

void Branch::remove(Term* term)
{
    assert_valid_term(term);
    ca_assert(term != NULL);
    remove(getIndex(term));
}

void Branch::remove(std::string const& name)
{
    if (!names.contains(name))
        return;

    Term* term = names[name];
    remove(getIndex(term));
}

void Branch::remove(int index)
{
    setNull(index);

    for (int i=index; i < _terms.length()-1; i++) {
        _terms[i] = _terms[i+1];
        if (_terms[i] != NULL)
            _terms[i]->index = i;
    }
    _terms.resize(_terms.length()-1);
}

void Branch::removeNulls()
{
    int numDeleted = 0;
    for (int i=0; i < _terms.length(); i++) {
        if (_terms[i] == NULL) {
            numDeleted++;
        } else if (numDeleted > 0) {
            _terms[i - numDeleted] = _terms[i];
            _terms[i - numDeleted]->index = i - numDeleted;
        }
    }

    if (numDeleted > 0)
        _terms.resize(_terms.length() - numDeleted);
}

void Branch::shorten(int newLength)
{
    if (newLength == 0) {
        clear();
        return;
    }

    for (int i=newLength; i < length(); i++)
        set(i, NULL);

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
    assert_valid_term(term);
    if (term->name != "" && term->name != name)
        throw std::runtime_error("term already has name: "+term->name);

    names.bind(term, name);
    term->name = name;
    update_unique_name(term);
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
    assert_valid_branch(this);

    for (int i=0; i < _terms.length(); i++) {
        _terms[i]->owningBranch = NULL;
        _terms[i]->index = 0;
    }

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

bool is_namespace(Term* term)
{
    return term->function == NAMESPACE_FUNC;
}

bool is_namespace(Branch& branch)
{
    return branch.owningTerm != NULL && is_namespace(branch.owningTerm);
}

std::string get_branch_source_filename(Branch& branch)
{
    Term* attr = branch["#attr:source-file"];
    if (attr == NULL)
        return "";
    else
        return as_string(attr);
}

Branch* get_outer_scope(Branch const& branch)
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

        // duplicate nested contents
        dest_term->nestedContents.clear();
        duplicate_branch_nested(newTermMap,
                source_term->nestedContents, dest_term->nestedContents);
    }
}

void duplicate_branch(Branch& source, Branch& dest)
{
    assert_valid_branch(&source);
    assert_valid_branch(&dest);

    ReferenceMap newTermMap;

    duplicate_branch_nested(newTermMap, source, dest);

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

    TaggedValue contents;
    storage::read_text_file_to_value(filename.c_str(), &contents, NULL);

    parser::compile(&branch, parser::statement_list, as_string(&contents));
}

void evaluate_script(Branch& branch, std::string const& filename)
{
    parse_script(branch, filename);
    evaluate_branch(branch);
}

void persist_branch_to_file(Branch& branch)
{
    std::string filename = get_branch_source_filename(branch);
    std::string contents = get_branch_source_text(branch) + "\n";
    storage::write_text_file(filename.c_str(), contents.c_str());
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
