// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/file.h"

#include "branch.h"
#include "building.h"
#include "kernel.h"
#include "code_iterators.h"
#include "dll_loading.h"
#include "evaluation.h"
#include "filesystem.h"
#include "file_utils.h"
#include "function.h"
#include "importing_macros.h"
#include "introspection.h"
#include "list.h"
#include "locals.h"
#include "parser.h"
#include "stateful_code.h"
#include "source_repro.h"
#include "static_checking.h"
#include "string_type.h"
#include "names.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"
#include "update_cascades.h"

namespace circa {

void assert_valid_branch(Branch const* obj)
{
    // this once did something
}

Branch::Branch()
  : owningTerm(NULL),
    inProgress(false),
    stateType(NULL),
    currentlyCascadingUpdates(false)
{
    gc_register_new_object((CircaObject*) this, &BRANCH_T, true);
}

Branch::~Branch()
{
    clear_branch(this);
    gc_on_object_deleted((CircaObject*) this);
}

void branch_list_references(CircaObject* object, GCReferenceList* list, GCColor color)
{
    Branch* branch = (Branch*) object;

    // Follow each term
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        gc_mark(list, (CircaObject*) term->type, color);
        gc_mark(list, (CircaObject*) term->nestedContents, color);
    }
}

void branch_setup_type(Type* type)
{
    type->name = name_from_string("Branch");
    type->gcListReferences = branch_list_references;
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
    ca_test_assert(index < length());
    return _terms[index];
}
Term* Branch::getSafe(int index)
{
    if (index >= length())
        return NULL;
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

    return term->index;
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
        if (_terms[i] != NULL)
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
            if ((_terms[i] == NULL) && i < list_length(&pendingUpdates))
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

Term* Branch::findFirstBinding(Name name)
{
    for (int i = 0; i < _terms.length(); i++) {
        if (_terms[i] == NULL)
            continue;
        if (_terms[i]->nameSymbol == name)
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
#ifndef TERM_HAS_SYMBOL
    term->name = name;
#endif
    term->nameSymbol = name_from_string(name.c_str());
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
    return parser::compile(this, parser::statement_list, code);
}

Term*
Branch::eval(std::string const& code)
{
    return parser::evaluate(this, parser::statement_list, code);
}

bool is_namespace(Term* term)
{
    return term->function == FUNCS.namespace_func;
}

bool is_namespace(Branch* branch)
{
    return branch->owningTerm != NULL && is_namespace(branch->owningTerm);
}

bool has_nested_contents(Term* term)
{
    return term->nestedContents != NULL;
}

Branch* nested_contents(Term* term)
{
    if (term->nestedContents == NULL) {
        term->nestedContents = new Branch();
        term->nestedContents->owningTerm = term;
    }
    return term->nestedContents;
}

void remove_nested_contents(Term* term)
{
    if (term->nestedContents == NULL)
        return;

    clear_branch(term->nestedContents);
    delete term->nestedContents;
    term->nestedContents = NULL;
}

caValue* branch_get_source_filename(Branch* branch)
{
    List* fileOrigin = branch_get_file_origin(branch);

    if (fileOrigin == NULL)
        return NULL;

    return fileOrigin->get(1);
}

std::string get_branch_source_filename(Branch* branch)
{
    caValue* val = branch_get_source_filename(branch);
    
    if (val == NULL || !is_string(val))
        return "";
    else
        return as_string(val);
}

Branch* get_outer_scope(Branch* branch)
{
    if (branch->owningTerm == NULL)
        return NULL;
    return branch->owningTerm->owningBranch;
}

void pre_erase_term(Term* term)
{
    // If this term declares a Type, then clear the Type.declaringTerm pointer
    // before it becomes invalid.
    if (is_type(term) && as_type(term_value(term))->declaringTerm == term)
        as_type(term_value(term))->declaringTerm = NULL;

    // Ditto for Function
    if (is_function(term) && as_function(term_value(term))->declaringTerm == term)
        as_function(term_value(term))->declaringTerm = NULL;
}

void erase_term(Term* term)
{
    assert_valid_term(term);

    pre_erase_term(term);

    set_null(term_value(term));
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
    set_null(&branch->pendingUpdates);

    branch->names.clear();
    branch->inProgress = false;

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

Term* find_term_by_id(Branch* branch, int id)
{
    for (BranchIterator it(branch); !it.finished(); it.advance()) {
        if (*it == NULL)
            continue;

        if (it->id == id)
            return *it;
    }

    return NULL;
}

void duplicate_branch_nested(TermMap& newTermMap, Branch* source, Branch* dest)
{
    // Duplicate every term
    for (int index=0; index < source->length(); index++) {
        Term* source_term = source->get(index);

        Term* dest_term = create_duplicate(dest, source_term, source_term->name, false);

        newTermMap[source_term] = dest_term;

        // duplicate nested contents
        clear_branch(nested_contents(dest_term));
        duplicate_branch_nested(newTermMap,
                nested_contents(source_term), nested_contents(dest_term));
    }
}

void duplicate_branch(Branch* source, Branch* dest)
{
    assert_valid_branch(source);
    assert_valid_branch(dest);

    TermMap newTermMap;

    duplicate_branch_nested(newTermMap, source, dest);

    // Remap pointers
    for (int i=0; i < dest->length(); i++)
        remap_pointers(dest->get(i), newTermMap);

    // Include/overwrite names
    dest->names.append(source->names);
    dest->names.remapPointers(newTermMap);
}

Name load_script(Branch* branch, const char* filename)
{
    // Store the file origin
    List* fileOrigin = set_list(&branch->origin, 3);
    set_name(fileOrigin->get(0), name_File);
    set_string(fileOrigin->get(1), filename);
    set_int(fileOrigin->get(2), circa_file_get_version(filename));

    // Read the text file
    const char* contents = circa_read_file(filename);

    if (contents == NULL) {
        Term* msg = create_string(branch, "file not found");
        apply(branch, STATIC_ERROR_FUNC, TermList(msg));
        return name_Failure;
    }

    parser::compile(branch, parser::statement_list, contents);

    post_module_load(branch);

    return name_Success;
}

void post_module_load(Branch* branch)
{
    // Post-load steps
    dll_loading_check_for_patches_on_loaded_branch(branch);
}

void evaluate_script(Branch* branch, const char* filename)
{
    load_script(branch, filename);
    evaluate_branch(branch);
}

Branch* include_script(Branch* branch, const char* filename)
{
    ca_assert(branch != NULL);
    Term* filenameTerm = create_string(branch, filename);
    Term* includeFunc = apply(branch, FUNCS.include_func, TermList(filenameTerm));
    return nested_contents(includeFunc);
}

Branch* load_script_term(Branch* branch, const char* filename)
{
    ca_assert(branch != NULL);
    Term* filenameTerm = create_string(branch, filename);
    Term* includeFunc = apply(branch, FUNCS.load_script, TermList(filenameTerm));
    return nested_contents(includeFunc);
}

#if 0
void save_script(Branch* branch)
{
    std::string text = get_branch_source_text(branch);
    List* fileOrigin = branch_get_file_origin(branch);
    if (fileOrigin == NULL)
        return;

    write_text_file(as_cstring(fileOrigin->get(1)), text.c_str());
}

void persist_branch_to_file(Branch* branch)
{
    std::string filename = get_branch_source_filename(branch);
    std::string contents = get_branch_source_text(branch);
    write_text_file(filename.c_str(), contents.c_str());
}
#endif

std::string get_source_file_location(Branch* branch)
{
    // Search upwards until we find a branch that has source-file defined.
    while (branch != NULL && get_branch_source_filename(branch) == "") {
        if (branch->owningTerm == NULL)
            branch = NULL;
        else
            branch = branch->owningTerm->owningBranch;
    }

    if (branch == NULL)
        return "";

    caValue* sourceFilename = branch_get_source_filename(branch);

    if (sourceFilename == NULL)
        return "";

    Value directory;
    circa_get_directory_for_filename(sourceFilename, &directory);

    return as_string(&directory);
}

List* branch_get_file_origin(Branch* branch)
{
    if (!is_list(&branch->origin))
        return NULL;

    List* list = (List*) &branch->origin;

    if (list->length() != 3)
        return NULL;

    if (as_name(list->get(0)) != name_File)
        return NULL;

    return list;
}

bool check_and_update_file_origin(Branch* branch, const char* filename)
{
    int version = circa_file_get_version(filename);

    List* fileOrigin = branch_get_file_origin(branch);

    if (fileOrigin == NULL) {
        fileOrigin = set_list(&branch->origin, 3);
        set_name(fileOrigin->get(0), name_File);
        set_string(fileOrigin->get(1), filename);
        set_int(fileOrigin->get(2), version);
        return true;
    }

    if (!equals_string(fileOrigin->get(1), filename)) {
        set_string(fileOrigin->get(1), filename);
        set_int(fileOrigin->get(2), version);
        return true;
    }

    if (!equals_int(fileOrigin->get(2), version)) {
        set_int(fileOrigin->get(2), version);
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

    mark_static_errors_invalid(branch);
    update_static_error_list(branch);
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

void branch_check_invariants(BranchInvariantCheck* result, Branch* branch)
{
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);

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
        if (term->owningBranch != branch)
            append_internal_error(result, i, "Wrong owningBranch");
    }
} 

bool branch_check_invariants_print_result(Branch* branch, std::ostream& out)
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
