// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/file.h"

#include "block.h"
#include "building.h"
#include "kernel.h"
#include "code_iterators.h"
#include "dll_loading.h"
#include "evaluation.h"
#include "file.h"
#include "function.h"
#include "importing_macros.h"
#include "inspection.h"
#include "list.h"
#include "names.h"
#include "native_modules.h"
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
#include "world.h"

namespace circa {

void on_block_created(Block* block)
{
    // No-op, used for debugging.
}

void assert_valid_block(Block const* obj)
{
    // this once did something
}

Block::Block()
  : owningTerm(NULL),
    version(0),
    inProgress(false),
    stateType(NULL),
    emptyEvaluation(false)
{
    id = global_world()->nextBlockID++;
    gc_register_new_object((CircaObject*) this, TYPES.block, true);

    on_block_created(this);
}

Block::~Block()
{
    clear_block(this);
    gc_on_object_deleted((CircaObject*) this);
}

Block* alloc_block_gc()
{
    Block* block = new Block();
    gc_mark_object_referenced(&block->header);
    gc_set_object_is_root(&block->header, false);
    return block;
}

void block_list_references(CircaObject* object, GCReferenceList* list, GCColor color)
{
    Block* block = (Block*) object;

    // Follow each term
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        gc_mark(list, (CircaObject*) term->type, color);
        gc_mark(list, (CircaObject*) term->nestedContents, color);
    }
}

std::string block_to_string(caValue* val)
{
    Block* block = as_block(val);
    if (block == NULL) {
        return "Block#null";
    } else {
        std::stringstream s;
        s << "Block#";
        s << block->id;
        return s.str();
    }
}

void block_setup_type(Type* type)
{
    set_string(&type->name, "Block");
    type->toString = block_to_string;
    type->gcListReferences = block_list_references;
}

int Block::length()
{
    assert_valid_block(this);
    return _terms.length();
}

bool Block::contains(std::string const& name)
{
    return get(name) != NULL;
}

Term* Block::get(int index)
{
    assert_valid_block(this);
    ca_test_assert(index < length());
    return _terms[index];
}
Term* Block::getSafe(int index)
{
    if (index >= length())
        return NULL;
    return _terms[index];
}

Term* Block::getFromEnd(int index)
{
    return get(length() - index - 1);
}

Term* Block::last()
{
    if (length() == 0) return NULL;
    else return _terms[length()-1];
}

int Block::getIndex(Term* term)
{
    ca_assert(term != NULL);
    ca_assert(term->owningBlock == this);
    assert_valid_term(term);

    return term->index;
}

void Block::append(Term* term)
{
    assert_valid_block(this);
    _terms.append(term);
    if (term != NULL) {
        assert_valid_term(term);
        ca_assert(term->owningBlock == NULL);
        term->owningBlock = this;
        term->index = _terms.length()-1;
    }
}

Term* Block::appendNew()
{
    assert_valid_block(this);
    Term* term = alloc_term();
    ca_assert(term != NULL);
    _terms.append(term);
    term->owningBlock = this;
    term->index = _terms.length()-1;
    return term;
}

void Block::set(int index, Term* term)
{
    assert_valid_block(this);
    ca_assert(index <= length());

    // No-op if this is the same term.
    if (_terms[index] == term)
        return;

    setNull(index);
    _terms.setAt(index, term);
    if (term != NULL) {
        assert_valid_term(term);
        ca_assert(term->owningBlock == NULL || term->owningBlock == this);
        term->owningBlock = this;
        term->index = index;
    }
}

void Block::setNull(int index)
{
    assert_valid_block(this);
    ca_assert(index <= length());
    Term* term = _terms[index];
    if (term != NULL)
        erase_term(term);
}

void Block::insert(int index, Term* term)
{
    assert_valid_term(term);
    assert_valid_block(this);
    ca_assert(index >= 0);
    ca_assert(index <= _terms.length());

    _terms.append(NULL);
    for (int i=_terms.length()-1; i > index; i--) {
        _terms.setAt(i, _terms[i-1]);
        _terms[i]->index = i;
    }
    _terms.setAt(index, term);

    if (term != NULL) {
        ca_assert(term->owningBlock == NULL);
        term->owningBlock = this;
        term->index = index;
    }
}

void Block::move(Term* term, int index)
{
    ca_assert(term->owningBlock == this);

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

void Block::moveToEnd(Term* term)
{
    assert_valid_term(term);
    ca_assert(term != NULL);
    ca_assert(term->owningBlock == this);
    ca_assert(term->index >= 0);
    int index = getIndex(term);
    _terms.append(term);
    _terms.setAt(index, NULL);
    term->index = _terms.length()-1;
}

void Block::remove(int index)
{
    remove_term(get(index));
}

void Block::remove(std::string const& name)
{
    if (!names.contains(name))
        return;

    Term* term = names[name];
    remove_term(term);
}

void Block::removeNulls()
{
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
}

void Block::removeNameBinding(Term* term)
{
    if (!has_empty_name(term) && names[term->name] == term)
        names.remove(term->name);
}

void Block::shorten(int newLength)
{
    for (int i=newLength; i < length(); i++)
        set(i, NULL);

    removeNulls();
}

void
Block::clear()
{
    clear_block(this);
}

Term* Block::findFirstBinding(Name name)
{
    for (int i = 0; i < _terms.length(); i++) {
        if (_terms[i] == NULL)
            continue;
        if (_terms[i]->nameSymbol == name)
            return _terms[i];
    }

    return NULL;
}

void Block::bindName(Term* term, Name name)
{
    assert_valid_term(term);
    if (!has_empty_name(term) && term->nameSymbol != name) {
        internal_error(std::string("term already has a name: ") + term->nameStr());
    }

    names.bind(term, name_to_string(name));
    term->nameSymbol = name;
    term->name = name_to_string(name);
    update_unique_name(term);
}

void Block::remapPointers(TermMap const& map)
{
    names.remapPointers(map);

    for (int i = 0; i < _terms.length(); i++) {
        Term* term = _terms[i];
        if (term != NULL)
            remap_pointers(term, map);
    }
}

std::string Block::toString()
{
    std::stringstream out;
    out << "[";
    for (int i=0; i < length(); i++) {
        Term* term = get(i);
        if (i > 0) out << ", ";
        if (!has_empty_name(term))
            out << term->nameStr() << ": ";
        out << term->toString();
    }
    out << "]";
    return out.str();
}

Term*
Block::compile(std::string const& code)
{
    return parser::compile(this, parser::statement_list, code);
}

Term*
Block::eval(std::string const& code)
{
    return parser::evaluate(this, parser::statement_list, code);
}

bool is_namespace(Term* term)
{
    return term->function == FUNCS.namespace_func;
}

bool is_namespace(Block* block)
{
    return block->owningTerm != NULL && is_namespace(block->owningTerm);
}

bool has_nested_contents(Term* term)
{
    return term->nestedContents != NULL;
}

Block* nested_contents(Term* term)
{
    if (term == NULL)
        return NULL;

    if (term->nestedContents == NULL) {
        term->nestedContents = new Block();
        term->nestedContents->owningTerm = term;
    }
    return term->nestedContents;
}

void remove_nested_contents(Term* term)
{
    if (term->nestedContents == NULL)
        return;

    Block* block = term->nestedContents;
    clear_block(term->nestedContents);

    // Delete this Block immediately, if it's not referenced.
    if (!block->header.referenced)
        delete term->nestedContents;

    term->nestedContents = NULL;
}

void block_graft_replacement(Block* target, Block* replacement)
{
    target->owningTerm->nestedContents = replacement;
    replacement->owningTerm = target->owningTerm;

    // Remove owningTerm link from existing block.
    target->owningTerm = NULL;
}

caValue* block_get_source_filename(Block* block)
{
    List* fileOrigin = block_get_file_origin(block);

    if (fileOrigin == NULL)
        return NULL;

    return fileOrigin->get(1);
}

Block* get_outer_scope(Block* block)
{
    if (block->owningTerm == NULL)
        return NULL;
    return block->owningTerm->owningBlock;
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
    remove_nested_contents(term);

    // for each user, clear that user's input list of this term
    remove_from_any_user_lists(term);
    clear_from_dependencies_of_users(term);

    if (term->owningBlock != NULL) {
        // remove name binding if necessary
        term->owningBlock->removeNameBinding(term);

        // index may be invalid if something bad has happened
        ca_assert(term->index < term->owningBlock->length());
        term->owningBlock->_terms.setAt(term->index, NULL);

        term->owningBlock = NULL;
        term->index = -1;
    }

    dealloc_term(term);
}

void clear_block(Block* block)
{
    assert_valid_block(block);
    set_null(&block->staticErrors);
    block->stateType = NULL;

    block->names.clear();
    block->inProgress = false;

    // Iterate through the block and tear down any term references, so that we
    // don't have to worry about stale pointers later.
    for (BlockIterator it(block); it.unfinished(); ++it) {
        if (*it == NULL)
            continue;

        pre_erase_term(*it);
        set_inputs(*it, TermList());
        remove_from_any_user_lists(*it);
        change_function(*it, NULL);
    }

    for (int i= block->_terms.length() - 1; i >= 0; i--) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;

        if (term->nestedContents)
            clear_block(term->nestedContents);
    }

    for (int i = block->_terms.length() - 1; i >= 0; i--) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;

        // Delete any leftover users, mark them as repairable.
        for (int userIndex = 0; userIndex < term->users.length(); userIndex++) {
            Term* user = term->users[userIndex];
            for (int depIndex = 0; depIndex < user->numDependencies(); depIndex++) {
                if (user->dependency(depIndex) == term) {
                    // mark_repairable_link(user, term->name, depIndex);
                    user->setDependency(depIndex, NULL);
                }
            }
        }

        erase_term(term);
    }

    block->_terms.clear();
}

Term* find_term_by_id(Block* block, int id)
{
    for (BlockIterator it(block); !it.finished(); it.advance()) {
        if (*it == NULL)
            continue;

        if (it->id == id)
            return *it;
    }

    return NULL;
}

void duplicate_block_nested(TermMap& newTermMap, Block* source, Block* dest)
{
    // Duplicate every term
    for (int index=0; index < source->length(); index++) {
        Term* source_term = source->get(index);

        Term* dest_term = create_duplicate(dest, source_term, source_term->name);

        newTermMap[source_term] = dest_term;

        // duplicate nested contents
        clear_block(nested_contents(dest_term));
        duplicate_block_nested(newTermMap,
                nested_contents(source_term), nested_contents(dest_term));
    }
}

void duplicate_block(Block* source, Block* dest)
{
    assert_valid_block(source);
    assert_valid_block(dest);

    TermMap newTermMap;

    duplicate_block_nested(newTermMap, source, dest);

    // Remap pointers
    for (int i=0; i < dest->length(); i++)
        remap_pointers(dest->get(i), newTermMap);

    // Include/overwrite names
    dest->names.append(source->names);
    dest->names.remapPointers(newTermMap);
}

Name load_script(Block* block, const char* filename)
{
    // Store the file origin
    caValue* origin = &block->origin;
    set_list(origin, 3);
    set_name(list_get(origin, 0), name_File);
    set_string(list_get(origin, 1), filename);
    set_int(list_get(origin, 2), circa_file_get_version(filename));

    // Read the text file
    circa::Value contents;
    circa_read_file(filename, &contents);

    if (is_null(&contents)) {
        Term* msg = create_string(block, "file not found");
        apply(block, FUNCS.static_error, TermList(msg));
        return name_Failure;
    }

    parser::compile(block, parser::statement_list, as_cstring(&contents));

    return name_Success;
}

Block* include_script(Block* block, const char* filename)
{
    ca_assert(block != NULL);
    Term* filenameTerm = create_string(block, filename);
    Term* includeFunc = apply(block, FUNCS.include_func, TermList(filenameTerm));
    return nested_contents(includeFunc);
}

Block* load_script_term(Block* block, const char* filename)
{
    ca_assert(block != NULL);
    Term* filenameTerm = create_string(block, filename);
    Term* includeFunc = apply(block, FUNCS.load_script, TermList(filenameTerm));
    return nested_contents(includeFunc);
}

std::string get_source_file_location(Block* block)
{
    // Search upwards until we find a block that has source-file defined.
    while (block != NULL && block_get_source_filename(block) == NULL)
        block = get_parent_block(block);

    if (block == NULL)
        return "";

    caValue* sourceFilename = block_get_source_filename(block);

    if (sourceFilename == NULL)
        return "";

    Value directory;
    get_directory_for_filename(sourceFilename, &directory);

    return as_string(&directory);
}

bool block_get_function_attr_bool(Block* block, Name attr)
{
    if (is_null(&block->functionAttrs))
        return false;

    // TODO

    return false;
}

List* block_get_file_origin(Block* block)
{
    if (!is_list(&block->origin))
        return NULL;

    List* list = (List*) &block->origin;

    if (list->length() != 3)
        return NULL;

    if (as_name(list->get(0)) != name_File)
        return NULL;

    return list;
}

bool check_and_update_file_origin(Block* block, const char* filename)
{
    int version = circa_file_get_version(filename);

    caValue* origin = block_get_file_origin(block);

    if (origin == NULL) {
        origin = &block->origin;
        set_list(origin, 3);
        set_name(list_get(origin, 0), name_File);
        set_string(list_get(origin, 1), filename);
        set_int(list_get(origin, 2), version);
        return true;
    }

    if (!equals_string(list_get(origin, 1), filename)) {
        touch(origin);
        set_string(list_get(origin, 1), filename);
        set_int(list_get(origin, 2), version);
        return true;
    }

    if (!equals_int(list_get(origin, 2), version)) {
        touch(origin);
        set_int(list_get(origin, 2), version);
        return true;
    }

    return false;
}

Block* load_latest_block(Block* block)
{
    caValue* fileOrigin = block_get_file_origin(block);
    if (fileOrigin == NULL)
        return block;

    std::string filename = as_string(list_get(fileOrigin, 1));

    bool fileChanged = check_and_update_file_origin(block, filename.c_str());

    if (!fileChanged)
        return block;

    Block* newBlock = alloc_block_gc();
    load_script(newBlock, filename.c_str());

    update_static_error_list(newBlock);

    // New block starts off with the old block's version, plus 1.
    newBlock->version = block->version + 1;

    return newBlock;
}

void append_internal_error(caValue* result, int index, std::string const& message)
{
    const int INTERNAL_ERROR_TYPE = 1;

    caValue* error = list_append(result);
    set_list(error, 3);
    set_int(list_get(error, 0), INTERNAL_ERROR_TYPE);
    set_int(list_get(error, 1), index);
    set_string(list_get(error, 2), message);
}

void block_check_invariants(caValue* result, Block* block)
{
    set_list(result, 0);

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

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

        // Check that owningBlock is correct
        if (term->owningBlock != block)
            append_internal_error(result, i, "Wrong owningBlock");
    }
} 

bool block_check_invariants_print_result(Block* block, std::ostream& out)
{
    circa::Value result;
    block_check_invariants(&result, block);

    if (list_length(&result) == 0)
        return true;

    out << list_length(&result) << " errors found in block " << &block
        << std::endl;

    for (int i=0; i < list_length(&result); i++) {
        caValue* error = list_get(&result,i);
        out << "[" << as_int(list_get(error, 1)) << "] ";
        out << as_cstring(list_get(error, 2));
        out << std::endl;
    }

    out << "contents:" << std::endl;
    print_block(block, out);

    return false;
}

void block_link_missing_functions(Block* block, Block* source)
{
    for (BlockIterator it(block); it.unfinished(); it.advance()) {
        Term* term = *it;
        if (term->function == NULL || term->function == FUNCS.unknown_function) {
            std::string funcName = term->stringProp("syntax:functionName", "");

            if (funcName == "")
                continue;

            // try to find this function
            Term* func = find_local_name(source, funcName.c_str(), name_LookupFunction);

            if (func != NULL)
                change_function(term, func);
        }
    }
}

} // namespace circa
