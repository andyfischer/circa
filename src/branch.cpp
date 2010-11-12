// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "debug_valid_objects.h"
#include "importing_macros.h"

namespace circa {

static void assert_valid_branch(Branch const* obj)
{
    debug_assert_valid_object((void*) obj, BRANCH_OBJECT);
}

Branch::Branch() : owningTerm(NULL), _refCount(0), registerCount(0), outputRegister(0)
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

namespace branch_t {
    void initialize(Type* type, TaggedValue* value)
    {
        set_pointer(value, new Branch());

        Branch& prototype = type->prototype;
        branch_t::branch_copy(prototype, as_branch(value));
    }

    void release(TaggedValue* value)
    {
        delete (Branch*) get_pointer(value);
        set_pointer(value, NULL);
    }

    void reset_to_prototype(TaggedValue* value)
    {
        Branch& branch = as_branch(value);
        branch.clear();
        Branch& prototype = value->value_type->prototype;
        branch_t::branch_copy(prototype, as_branch(value));
    }

    void copy(TaggedValue* sourceValue, TaggedValue* destValue)
    {
        Branch& source = as_branch(sourceValue);
        Branch& dest = as_branch(destValue);
        assert_valid_branch(&source);
        assert_valid_branch(&dest);

        branch_copy(source, dest);
    }

    void cast(Type*, TaggedValue* sourceValue, TaggedValue* destValue)
    {
        Branch& dest = as_branch(destValue);
        assert_valid_branch(&dest);

        if (is_branch(sourceValue)) {
            Branch& source = as_branch(sourceValue);
            assert_valid_branch(&source);

            // For Branch or List type, overwrite existing shape
            if (is_branch(destValue) || (list_t::is_list(destValue)))
                branch_copy(source, dest);
            else
                assign(source, dest);
        } else {
            dest.clear();
            int numElements = sourceValue->numElements();
            for (int i=0; i < numElements; i++) {
                Term* v = create_value(dest, ANY_TYPE);
                circa::copy(sourceValue->getIndex(i), v);
            }
        }
    }

    TaggedValue* get_index(TaggedValue* value, int index)
    {
        Branch& b = as_branch(value);
        if (index >= b.length())
            return NULL;
        return b[index];
    }

    void set_index(TaggedValue* value, int index, TaggedValue* element)
    {
        ca_assert(value != element);
        circa::copy(element, as_branch(value)[index]);
    }

    TaggedValue* get_field(TaggedValue* value, const char* name)
    {
        Branch& b = as_branch(value);
        return b[name];
    }

    void set_field(TaggedValue* value, const char* name, TaggedValue* element)
    {
        TaggedValue* destination = as_branch(value)[name];
        if (destination == NULL)
            return;
        ca_assert(destination != value);
        circa::copy(element, as_branch(value)[name]);
    }

    int num_elements(TaggedValue* value)
    {
        Branch& b = as_branch(value);
        return b.length();
    }

    void branch_copy(Branch& source, Branch& dest)
    {
        assert_valid_branch(&source);
        assert_valid_branch(&dest);

        // Assign terms as necessary
        int lengthToAssign = std::min(source.length(), dest.length());

        for (int i=0; i < lengthToAssign; i++) {
            assert_valid_term(source[i]);
            assert_valid_term(dest[i]);

            // Change type if needed
            if (source[i]->type != dest[i]->type)
                change_type(source[i], dest[i]->type);
            circa::copy(source[i], dest[i]);
        }

        // Add terms if necessary
        for (int i=dest.length(); i < source.length(); i++) {
            ca_assert(source[i] != NULL);
            assert_valid_term(source[i]);

            Term* t = create_duplicate(dest, source[i]);
            if (source[i]->name != "")
                dest.bindName(t, source[i]->name);
        }

        // Remove terms if necessary
        for (int i=source.length(); i < dest.length(); i++) {
            dest.set(i, NULL);
        }

        dest.removeNulls();
    }

    void assign(Branch& source, Branch& dest)
    {
        // Temporary special case, if the two branches have different sizes then
        // do a copy instead. This should be removed.
        if (source.length() != dest.length())
            return branch_copy(source, dest);

        for (int i=0; i < source.length(); i++)
            cast(source[i], dest[i]);
    }

    bool equals(TaggedValue* lhsValue, TaggedValue* rhs)
    {
        if (rhs->value_type->numElements == NULL
            || rhs->value_type->getIndex == NULL)
            return false;

        Branch& lhs = as_branch(lhsValue);
    
        if (lhs.length() != rhs->numElements())
            return false;

        for (int i=0; i < lhs.length(); i++) {
            if (!circa::equals(lhs[i], rhs->getIndex(i)))
                return false;
        }

        return true;
    }
    CA_FUNCTION(append)
    {
        circa::copy(INPUT(0), OUTPUT);
        Branch& branch = as_branch(OUTPUT);
        Term* t = INPUT_TERM(1);
        create_duplicate(branch, t);
    }
    void setup_type(Term* type)
    {
        Term* branch_append = 
            import_member_function(type, append, "append(Branch, any) -> Branch");
        function_set_use_input_as_output(branch_append, 0, true);
    }
}

bool is_branch(TaggedValue* value)
{
    return is_branch_based_type(value->value_type);
}

Branch& as_branch(TaggedValue* value)
{
    ca_assert(value != NULL);
    ca_assert(is_branch(value));
    return *((Branch*) value->value_data.ptr);
}

Branch& as_branch(Term* term)
{
    ca_assert(term->nestedContents.length() == 0); // <- Temp while things are refactored
    return as_branch((TaggedValue*) term);
}

std::string compound_type_to_string(TaggedValue* value)
{
    std::stringstream out;
    out << "[";

    Branch& branch = as_branch(value);

    for (int i=0; i < branch.length(); i++) {
        if (i != 0)
            out << ", ";
        out << to_string(branch[i]);
    }

    out << "]";
    return out.str();
}

bool is_branch_based_type(Term* type)
{
    ca_assert(type != NULL);
    ca_assert(type_contents(type) != NULL);
    return type_contents(type)->initialize == branch_t::initialize;
}

bool is_branch_based_type(Type* type)
{
    ca_assert(type != NULL);
    return type->initialize == branch_t::initialize;
}

void initialize_branch_based_type(Term* term)
{
    Type* type = &as_type(term);

    reset_type(type);
    type->name = "Branch";
    type->initialize = branch_t::initialize;
    type->release = branch_t::release;
    type->copy = branch_t::copy;
    type->reset = branch_t::reset_to_prototype;
    type->cast = branch_t::cast;
    type->equals = branch_t::equals;
    type->getIndex = branch_t::get_index;
    type->setIndex = branch_t::set_index;
    type->getField = branch_t::get_field;
    type->setField = branch_t::set_field;
    type->numElements = branch_t::num_elements;
    type->toString = compound_type_to_string;
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

    update_register_indices(dest);
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

    std::string fileContents = storage::read_text_file_as_str(filename.c_str());

    parser::compile(&branch, parser::statement_list, fileContents);
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

bool branch_check_invariants(Branch& branch, std::ostream* output)
{
    bool success = true;

    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term == NULL) continue;
        
        // Check that the term's index is correct
        if (term->index != i) {
            success = false;
            if (output != NULL) {
                *output << get_short_location(term) << " has wrong index: found "<<term->index
                   << ", should be " << i << std::endl;
            }
        }

        // Check that owningBranch is correct
        if (term->owningBranch != &branch) {
            success = false;
            if (output != NULL) {
                *output << get_short_location(term) << " has wrong owningBranch: found"
                    << term->owningBranch << ", should be " << &branch << std::endl;
            }
        }

        // Run check_invariants on the term
        if (term != NULL) {
            std::string str;
            bool result = check_invariants(term, str);
            if (!result) {
                success = false;
                if (output != NULL)
                    *output << get_short_location(term) << " " << str << std::endl;
            }
        }
    }
    return success;
}

} // namespace circa
