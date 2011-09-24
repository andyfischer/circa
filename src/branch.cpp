// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "builtins.h"
#include "code_iterators.h"
#include "evaluation.h"
#include "filesystem.h"
#include "function.h"
#include "heap_debugging.h"
#include "importing_macros.h"
#include "introspection.h"
#include "list_shared.h"
#include "locals.h"
#include "parser.h"
#include "refactoring.h"
#include "stateful_code.h"
#include "source_repro.h"
#include "static_checking.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"
#include "update_cascades.h"

namespace circa {

void assert_valid_branch(Branch const* obj)
{
    debug_assert_valid_object((void*) obj, BRANCH_OBJECT);
}

Branch::Branch()
  : owningTerm(NULL),
    _refCount(0),
    outputIndex(0),
    inuse(false),
    currentlyCascadingUpdates(false)
{
    debug_register_valid_object((void*) this, BRANCH_OBJECT);
}

Branch::~Branch()
{
    clear_branch(this);
    debug_unregister_valid_object(this, BRANCH_OBJECT);
}

int Branch::length()
{
    assert_valid_branch(this);
    return _terms.length();
}

bool Branch::contains(std::string const& name)
{
    return get(name) != NULL;
}

Term* Branch::get(int index)
{
    assert_valid_branch(this);
    if (index >= length())
        throw std::runtime_error("index out of range");
    return _terms[index];
}

Term* Branch::getFromEnd(int index)
{
    return get(length() - index - 1);
}

Term* Branch::last()
{
    if (length() == 0) return NULL;
    else return _terms[length()-1];
}

int Branch::getIndex(Term* term)
{
    ca_assert(term != NULL);
    ca_assert(term->owningBranch == this);
    assert_valid_term(term);

    //assert(term->index == debugFindIndex(term));

    return term->index;
}

int Branch::debugFindIndex(Term* term)
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
int Branch::findIndex(const char* name)
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

void Branch::set(int index, Term* term)
{
    assert_valid_branch(this);
    ca_assert(index <= length());

    // No-op if this is the same term.
    if (_terms[index] == term)
        return;

    setNull(index);
    _terms.setAt(index, term);
    if (term != NULL) {
        assert_valid_term(term);
        ca_assert(term->owningBranch == NULL || term->owningBranch == this);
        term->owningBranch = this;
        term->index = index;
    }
}

void Branch::setNull(int index)
{
    assert_valid_branch(this);
    ca_assert(index <= length());
    Term* term = _terms[index];
    if (term != NULL)
        erase_term(term);
}

void Branch::insert(int index, Term* term)
{
    assert_valid_term(term);
    assert_valid_branch(this);
    ca_assert(index >= 0);
    ca_assert(index <= _terms.length());

    _terms.append(NULL);
    for (int i=_terms.length()-1; i > index; i--) {
        _terms.setAt(i, _terms[i-1]);
        _terms[i]->index = i;
    }
    _terms.setAt(index, term);

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

    for (int i=term->index; i != index; i += dir) {
        _terms.setAt(i, _terms[i+dir]);
        _terms[i]->index = i;
    }
    _terms.setAt(index, term);
    term->index = index;
}

void Branch::moveToEnd(Term* term)
{
    assert_valid_term(term);
    ca_assert(term != NULL);
    ca_assert(term->owningBranch == this);
    ca_assert(term->index >= 0);
    int index = getIndex(term);
    _terms.append(term);
    _terms.setAt(index, NULL);
    term->index = _terms.length()-1;
}

void Branch::remove(int index)
{
    remove_term(get(index));
}

void Branch::remove(std::string const& name)
{
    if (!names.contains(name))
        return;

    Term* term = names[name];
    remove_term(term);
}

void Branch::removeNulls()
{
    if (is_list(&pendingUpdates)) {
        for (int i=0; i < _terms.length(); i++)
            if ((_terms[i] == NULL) && i < list_get_length(&pendingUpdates))
                list_remove_index(&pendingUpdates, i);
    }

    int numDeleted = 0;
    for (int i=0; i < _terms.length(); i++) {
        if (_terms[i] == NULL) {
            numDeleted++;
        } else if (numDeleted > 0) {
            _terms.setAt(i - numDeleted, _terms[i]);
            _terms[i - numDeleted]->index = i - numDeleted;
        }
    }

    if (numDeleted > 0)
        _terms.resize(_terms.length() - numDeleted);

    if (is_list(&pendingUpdates))
        List::checkCast(&pendingUpdates)->resize(_terms.length());
}
void Branch::removeNameBinding(Term* term)
{
    if (term->name != "" && names[term->name] == term)
        names.remove(term->name);
}

void Branch::shorten(int newLength)
{
    if (newLength == 0) {
        clear_branch(this);
        return;
    }

    for (int i=newLength; i < length(); i++)
        set(i, NULL);

    removeNulls();
}

void
Branch::clear()
{
    clear_branch(this);
}

Term* Branch::findFirstBinding(std::string const& name)
{
    for (int i = 0; i < _terms.length(); i++) {
        if (_terms[i] == NULL)
            continue;
        if (_terms[i]->name == name)
            return _terms[i];
    }

    return NULL;
}

Term* Branch::findLastBinding(std::string const& name)
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

void Branch::remapPointers(TermMap const& map)
{
    names.remapPointers(map);

    for (int i = 0; i < _terms.length(); i++) {
        Term* term = _terms[i];
        if (term != NULL)
            remap_pointers(term, map);
    }
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
    return parser::compile(*this, parser::statement_list, code);
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

bool has_nested_contents(Term* term)
{
    return term->nestedContents != NULL;
}

Branch& nested_contents(Term* term)
{
    if (term->nestedContents == NULL) {
        term->nestedContents = new Branch();
        term->nestedContents->owningTerm = term;
    }
    return *term->nestedContents;
}

std::string get_branch_source_filename(Branch& branch)
{
    List* fileOrigin = branch_get_file_origin(&branch);

    if (fileOrigin == NULL)
        return "";

    return as_string(fileOrigin->get(1));
}

Branch* get_outer_scope(Branch const& branch)
{
    if (branch.owningTerm == NULL)
        return NULL;
    return branch.owningTerm->owningBranch;
}

void pre_erase_term(Term* term)
{
    // If this term declares a Type, then clear the Type.declaringTerm pointer
    // before it becomes invalid.
    if (is_value(term) && is_type(term) && as_type(term)->declaringTerm == term)
        as_type(term)->declaringTerm = NULL;

    // Ditto for Function
    if (is_value(term) && is_function(term) && get_function_attrs(term)->declaringTerm == term)
        get_function_attrs(term)->declaringTerm = NULL;
}

void erase_term(Term* term)
{
    assert_valid_term(term);

    pre_erase_term(term);

    set_null((TaggedValue*) term);
    set_inputs(term, TermList());
    change_function(term, NULL);
    term->type = NULL;
    if (term->nestedContents) {
        clear_branch(term->nestedContents);
        delete term->nestedContents;
        term->nestedContents = NULL;
    }

    // for each user, clear that user's input list of this term
    remove_from_any_user_lists(term);
    clear_from_dependencies_of_users(term);

    if (term->owningBranch != NULL) {
        // remove name binding if necessary
        term->owningBranch->removeNameBinding(term);

        // index may be invalid if something bad has happened
        ca_assert(term->index < term->owningBranch->length());
        term->owningBranch->_terms.setAt(term->index, NULL);

        term->owningBranch = NULL;
        term->index = -1;
    }

    dealloc_term(term);
}

void clear_branch(Branch* branch)
{
    assert_valid_branch(branch);
    set_null(&branch->staticErrors);
    mark_branch_as_possibly_not_having_inlined_state(*branch);
    set_null(&branch->pendingUpdates);

    branch->names.clear();

    // Iterate through the branch and tear down any term references, so that we
    // don't have to worry about stale pointers later.
    for (BranchIterator it(branch); it.unfinished(); ++it) {
        if (*it == NULL)
            continue;

        pre_erase_term(*it);
        set_inputs(*it, TermList());
        remove_from_any_user_lists(*it);
        change_function(*it, NULL);
    }

    for (int i= branch->_terms.length() - 1; i >= 0; i--) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;

        if (term->nestedContents)
            clear_branch(term->nestedContents);
    }

    for (int i = branch->_terms.length() - 1; i >= 0; i--) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;

        // Delete any leftover users, mark them as repairable.
        for (int userIndex = 0; userIndex < term->users.length(); userIndex++) {
            Term* user = term->users[userIndex];
            for (int depIndex = 0; depIndex < user->numDependencies(); depIndex++) {
                if (user->dependency(depIndex) == term) {
                    mark_repairable_link(user, term->name, depIndex);
                    user->setDependency(depIndex, NULL);
                }
            }
        }

        erase_term(term);
    }

    branch->_terms.clear();
}

Term* find_term_by_id(Branch& branch, unsigned int id)
{
    for (BranchIterator it(&branch); !it.finished(); it.advance()) {
        if (*it == NULL)
            continue;

        if (it->globalID == id)
            return *it;
    }

    return NULL;
}

void duplicate_branch_nested(TermMap& newTermMap, Branch& source, Branch& dest)
{
    // Duplicate every term
    for (int index=0; index < source.length(); index++) {
        Term* source_term = source.get(index);

        Term* dest_term = create_duplicate(dest, source_term, source_term->name, false);

        newTermMap[source_term] = dest_term;

        // duplicate nested contents
        clear_branch(&nested_contents(dest_term));
        duplicate_branch_nested(newTermMap,
                nested_contents(source_term), nested_contents(dest_term));
    }
}

void duplicate_branch(Branch& source, Branch& dest)
{
    assert_valid_branch(&source);
    assert_valid_branch(&dest);

    TermMap newTermMap;

    duplicate_branch_nested(newTermMap, source, dest);

    // Remap pointers
    for (int i=0; i < dest.length(); i++)
        remap_pointers(dest[i], newTermMap);

    // Include/overwrite names
    dest.names.append(source.names);
    dest.names.remapPointers(newTermMap);
}

void load_script(Branch* branch, const char* filename)
{
    // Store the file origin
    List* fileOrigin = set_list(&branch->origin, 3);
    copy(&FILE_SYMBOL, fileOrigin->get(0));
    set_string(fileOrigin->get(1), filename);
    set_int(fileOrigin->get(2), get_modified_time(filename));

    // Read the text file
    TaggedValue contents;
    TaggedValue fileReadError;
    read_text_file_to_value(filename, &contents, &fileReadError);

    if (!is_null(&fileReadError)) {
        Term* msg = create_value(*branch, &fileReadError, "fileReadError");
        apply(*branch, STATIC_ERROR_FUNC, TermList(msg));
        return;
    }

    parser::compile(*branch, parser::statement_list, as_string(&contents));
}

void evaluate_script(Branch& branch, const char* filename)
{
    load_script(&branch, filename);
    evaluate_branch(branch);
}

Branch* include_script(Branch* branch, const char* filename)
{
    ca_assert(branch != NULL);
    Term* filenameTerm = create_string(*branch, filename);
    Term* includeFunc = apply(*branch, INCLUDE_FUNC, TermList(filenameTerm));
    post_compile_term(includeFunc);
    return &nested_contents(includeFunc);
}

Branch* load_script_term(Branch* branch, const char* filename)
{
    ca_assert(branch != NULL);
    Term* filenameTerm = create_string(*branch, filename);
    Term* includeFunc = apply(*branch, LOAD_SCRIPT_FUNC, TermList(filenameTerm));
    post_compile_term(includeFunc);
    return &nested_contents(includeFunc);
}

void persist_branch_to_file(Branch& branch)
{
    std::string filename = get_branch_source_filename(branch);
    std::string contents = get_branch_source_text(branch);
    write_text_file(filename.c_str(), contents.c_str());
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

List* branch_get_file_origin(Branch* branch)
{
    if (!is_list(&branch->origin))
        return NULL;

    List* list = (List*) &branch->origin;

    if (list->length() != 3)
        return NULL;

    if (!equals(list->get(0), &FILE_SYMBOL))
        return NULL;

    return list;
}

bool check_and_update_file_origin(Branch* branch, const char* filename)
{
    time_t modifiedTime = get_modified_time(filename);

    List* fileOrigin = branch_get_file_origin(branch);

    if (fileOrigin == NULL) {
        fileOrigin = set_list(&branch->origin, 3);
        copy(&FILE_SYMBOL, fileOrigin->get(0));
        set_string(fileOrigin->get(1), filename);
        set_int(fileOrigin->get(2), modifiedTime);
        return true;
    }

    if (!equals_string(fileOrigin->get(1), filename)) {
        set_string(fileOrigin->get(1), filename);
        set_int(fileOrigin->get(2), modifiedTime);
        return true;
    }

    if (!equals_int(fileOrigin->get(2), modifiedTime)) {
        set_int(fileOrigin->get(2), modifiedTime);
        return true;
    }

    return false;
}

bool refresh_script(Branch* branch)
{
    List* fileOrigin = branch_get_file_origin(branch);
    if (fileOrigin == NULL)
        return false;

    std::string filename = as_string(fileOrigin->get(1));

    bool fileChanged = check_and_update_file_origin(branch, filename.c_str());

    if (!fileChanged)
        return false;

    clear_branch(branch);
    load_script(branch, filename.c_str());

#if 0 // TODO: Logic for exposing names
    Branch* parent = get_parent_branch(branch);

    if (caller->owningBranch != NULL && exposeNames) {
        expose_all_names(contents, *caller->owningBranch);
        finish_update_cascade(*caller->owningBranch);
    }
#endif

    mark_static_errors_invalid(*branch);
    update_static_error_list(*branch);
    return true;
}

void append_internal_error(BranchInvariantCheck* result, int index, std::string const& message)
{
    const int INTERNAL_ERROR_TYPE = 1;

    List& error = *set_list(result->errors.append(), 3);
    set_int(error[0], INTERNAL_ERROR_TYPE);
    set_int(error[1], index);
    set_string(error[2], message);
}

void branch_check_invariants(BranchInvariantCheck* result, Branch& branch)
{
    int expectedLocalIndex = 0;

    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];

        if (term == NULL) {
            append_internal_error(result, i, "NULL pointer");
            continue;
        }

        // Check that the term's index is correct
        if (term->index != i) {
            std::stringstream msg;
            msg << "Wrong index (found " << term->index << ", expected " << i << ")";
            append_internal_error(result, i, msg.str());
        }

        // Check that owningBranch is correct
        if (term->owningBranch != &branch)
            append_internal_error(result, i, "Wrong owningBranch");

        // Check localIndex
        int numOutputs = get_output_count(term);
        if (numOutputs != 0) {
            if (term->localsIndex != expectedLocalIndex) {
                std::stringstream msg;
                msg << "Wrong localsIndex (found " << term->localsIndex << ", expected "
                    << expectedLocalIndex << ")";
                append_internal_error(result, i, msg.str());
            }
            expectedLocalIndex = term->localsIndex + numOutputs;
        }
    }
} 

bool branch_check_invariants_print_result(Branch& branch, std::ostream& out)
{
    BranchInvariantCheck result;
    branch_check_invariants(&result, branch);

    if (result.errors.length() == 0)
        return true;

    out << result.errors.length() << " errors found in branch " << &branch
        << std::endl;

    for (int i=0; i < result.errors.length(); i++) {
        List* error = List::checkCast(result.errors[i]);
        out << "[" << error->get(1)->asInt() << "] ";
        out << error->get(2)->asString();
        out << std::endl;
    }

    out << "contents:" << std::endl;
    print_branch(out, branch);

    return false;
}

} // namespace circa
