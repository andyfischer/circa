// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "function.h"
#include "importing_macros.h"
#include "kernel.h"
#include "inspection.h"
#include "parser.h"
#include "source_repro.h"
#include "static_checking.h"
#include "string_type.h"
#include "names.h"
#include "tagged_value.h"
#include "term.h"
#include "token.h"
#include "type.h"

#include "types/int.h"
#include "types/common.h"

namespace circa {

Term* IMPLICIT_TYPES = NULL;

namespace type_t {

    void initialize(Type*, caValue* value)
    {
        Type* type = create_type();
        set_pointer(value, type);
    }
    void copy(Type* type, caValue* source, caValue* dest)
    {
        // Shallow copy
        ca_assert(is_type(source));
        change_type(dest, type);
        dest->value_data = source->value_data;
    }

    void type_gc_release(CircaObject* obj)
    {
        Type* type = (Type*) obj;
        delete type;
    }

    void formatSource(caValue* source, Term* term)
    {
        append_phrase(source, "type ", term, name_Keyword);
        append_phrase(source, term->name, term, name_TypeName);

        if (term->boolProp("syntax:SuperSpecialHandleType", false)) {
            append_phrase(source, " = handle_type()", term, name_None);
        }

        if (term->boolProp("syntax:noBrackets", false))
            return;

        append_phrase(source, term->stringProp("syntax:preLBracketWhitespace", " "),
                term, tok_Whitespace);
        append_phrase(source, "{", term, tok_LBracket);
        append_phrase(source, term->stringProp("syntax:postLBracketWhitespace", " "),
                term, tok_Whitespace);

        Block* contents = nested_contents(term);

        for (int i=0; i < contents->length(); i++) {
            Term* field = contents->get(i);

            if (is_comment(field)) {
                append_phrase(source, field->stringProp("comment",""), field, tok_Comment);
                append_phrase(source, field->stringProp("syntax:lineEnding",""), field, tok_Whitespace);
                continue;
            }
            ca_assert(field != NULL);
            append_phrase(source, field->stringProp("syntax:preWhitespace",""),
                    term, tok_Whitespace);

            Type* fieldType = function_get_output_type(field, 0);
            append_phrase(source, as_cstring(&fieldType->name), term, name_TypeName);
            append_phrase(source, field->stringProp("syntax:postNameWs"," "),
                    term, tok_Whitespace);
            append_phrase(source, field->name, term, tok_Identifier);
            append_phrase(source, field->stringProp("syntax:postWhitespace",""),
                    term, tok_Whitespace);
        }
        append_phrase(source, "}", term, tok_RBracket);
    }

    std::string toString(caValue* value)
    {
        return std::string("<Type ")+ as_cstring(&as_type(value)->name)+">";
    }
    void setup_type(Type* type)
    {
        set_string(&type->name, "Type");
        type->storageType = name_StorageTypeType;
        type->initialize = type_t::initialize;
        type->copy = copy;
        type->gcRelease = type_gc_release;
        type->formatSource = formatSource;
        type->toString = toString;
    }

} // namespace type_t

Type::Type()
{
    memset(this, 0, sizeof(*this));
    initialize_type(this);
}

Type::~Type()
{
    gc_on_object_deleted((CircaObject*) this);
}

const char*
Type::nameStr()
{
    return as_cstring(&name);
}

Type* get_output_type(Term* term, int outputIndex)
{
    if (outputIndex == 0)
        return term->type;

    if (term->function == NULL)
        return TYPES.any;

    return function_get_output_type(term->function, outputIndex);
}

Type* get_output_type(Term* term)
{
    return get_output_type(term, 0);
}

Type* get_type_of_input(Term* term, int inputIndex)
{
    if (inputIndex >= term->numInputs())
        return NULL;
    if (term->input(inputIndex) == NULL)
        return NULL;
    return get_output_type(term->input(inputIndex), 0);
}

caValue* get_type_property(Type* type, const char* name)
{
    return type->properties[name];
}

caValue* type_property_insert(Type* type, const char* name)
{
    return type->properties.insert(name);
}

void set_type_property(Type* type, const char* name, caValue* value)
{
    copy(value, type->properties.insert(name));
}

Block* type_declaration_block(Type* type)
{
    if (type->declaringTerm == NULL)
        return NULL;
    return type->declaringTerm->nestedContents;
}

Type* create_type_uninitialized()
{
    Type* t = (Type*) malloc(sizeof(Type));
    memset(t, 0, sizeof(Type));
    return t;
}

void initialize_type(Type* t)
{
    t->storageType = name_StorageTypeNull;

    initialize_null(&t->properties);
    set_dict(&t->properties);

    initialize_null(&t->parameter);
    initialize_null(&t->name);
    set_string(&t->name, "");

    gc_register_new_object((CircaObject*) t, TYPES.type, true);
}

Type* create_type()
{
    Type* t = create_type_uninitialized();
    initialize_type(t);
    gc_set_object_is_root((CircaObject*) t, false);
    return t;
}

Type* unbox_type(Term* term)
{
    return as_type(term);
}

Type* unbox_type(caValue* val)
{
    return as_type(val);
}

static void run_static_type_query(StaticTypeQuery* query)
{
    // Check that the subject term and subjectType match.
    if (query->subject && query->subjectType) {
        ca_assert(query->subjectType == declared_type(query->subject));
	}

    // If the subject has a NULL type then just fail early. This should only
    // happen when deleting terms.
    if (query->subject && declared_type(query->subject) == NULL)
        return query->fail();

    ca_assert(query->type);
    ca_assert(query->subject == NULL || declared_type(query->subject));

    // Check that either subject or subjectType are provided.
    ca_assert(query->subjectType || query->subject);

    // Populate subjectType from subject if missing.
    if (query->subjectType == NULL)
        query->subjectType = declared_type(query->subject);

    // If output term is ANY type then we cannot statically determine.
    if (query->subjectType == TYPES.any)
        return query->unableToDetermine();

    // Always succeed if types are the same.
    if (query->subjectType == query->type)
        return query->succeed();

    // Try using the type's static query func
    Type::StaticTypeQueryFunc staticTypeQueryFunc = query->type->staticTypeQuery;
    if (staticTypeQueryFunc != NULL) {
        staticTypeQueryFunc(query->type, query);
        return;
    }

    // No static query function, and we know that the types are not equal, so
    // default behavior here is to fail.
    return query->fail();
}

StaticTypeQuery::Result run_static_type_query(Type* type, Type* subjectType)
{
    StaticTypeQuery query;
    query.type = type;
    query.subjectType = subjectType;
    run_static_type_query(&query);
    return query.result;
}
StaticTypeQuery::Result run_static_type_query(Type* type, Term* term)
{
    StaticTypeQuery query;
    query.type = type;
    query.subject = term;
    run_static_type_query(&query);
    return query.result;
}

bool term_output_always_satisfies_type(Term* term, Type* type)
{
    ca_assert(term != NULL);
    ca_assert(type != NULL);
    return run_static_type_query(type, term) == StaticTypeQuery::SUCCEED;
}

bool term_output_never_satisfies_type(Term* term, Type* type)
{
    return run_static_type_query(type, term) == StaticTypeQuery::FAIL;
}

bool type_is_static_subset_of_type(Type* superType, Type* subType)
{
    StaticTypeQuery query;
    query.type = superType;
    query.subjectType = subType;
    run_static_type_query(&query);
    return query.result != StaticTypeQuery::FAIL;
}

void reset_type(Type* type)
{
    type->storageType = name_StorageTypeNull;
    type->initialize = NULL;
    type->release = NULL;
    type->copy = NULL;
    type->reset = NULL;
    type->equals = NULL;
    type->cast = NULL;
    type->staticTypeQuery = NULL;
    type->toString = NULL;
    type->formatSource = NULL;
    type->touch = NULL;
    type->getIndex = NULL;
    type->setIndex = NULL;
    type->getField = NULL;
    type->setField = NULL;
    type->numElements = NULL;
    type->checkInvariants = NULL;
    type->remapPointers = NULL;
    type->hashFunc = NULL;
    type->visitHeap = NULL;

    clear_type_contents(type);
}

void clear_type_contents(Type* type)
{
    set_null(&type->parameter);
}

void initialize_simple_pointer_type(Type* type)
{
    reset_type(type);
}

void type_initialize_kernel(Block* kernel)
{
    IMPLICIT_TYPES = create_block(kernel, "#implicit_types")->owningTerm;
}

Term* create_tuple_type(caValue* types)
{
    std::stringstream typeName;
    typeName << "Tuple<";
    for (int i=0; i < list_length(types); i++) {
        if (i != 0) typeName << ",";
        typeName << as_cstring(&as_type(list_get(types,i))->name);
    }
    typeName << ">";

    Term* result = create_type(nested_contents(IMPLICIT_TYPES), typeName.str().c_str());
    list_t::setup_type(unbox_type(result));

    unbox_type(result)->parent = TYPES.list;

    caValue* parameter = set_list(&unbox_type(result)->parameter, list_length(types));

    for (int i=0; i < list_length(types); i++) {
        ca_assert(is_type(list_get(types,i)));
        set_type(list_get(parameter,i), as_type(list_get(types,i)));
    }
    
    return result;
}

std::string get_base_type_name(std::string const& typeName)
{
    size_t pos = typeName.find_first_of("<");
    if (pos != std::string::npos)
        return typeName.substr(0, pos);
    return "";
}

Term* find_method_with_search_name(Block* block, Type* type, const char* searchName)
{
    // Local name search.
    if (block != NULL) {
        Term* term = find_name(block, searchName);
        if (term != NULL && is_function(term))
            return term;
    }

    // If not found, look in the block where the type was declared.
    Block* typeDeclarationBlock = NULL;

    if (type->declaringTerm != NULL)
        typeDeclarationBlock = type->declaringTerm->owningBlock;

    if (typeDeclarationBlock != NULL && typeDeclarationBlock != block) {
        Term* term = find_name(typeDeclarationBlock, searchName);
        if (term != NULL && is_function(term))
            return term;
    }

    return NULL;
}

Term* find_method(Block* block, Type* type, const char* name)
{
    if (string_eq(&type->name, ""))
        return NULL;

    // First, look inside the type definition, which contains simulated methods and
    // possibly other stuff.
    Block* typeDef = type_declaration_block(type);
    if (typeDef != NULL) {
        Term* func = find_local_name(typeDef, name);
        if (func != NULL && is_function(func))
            return func;
    }

    // Construct the search name, which looks like TypeName.functionName.
    std::string searchName = std::string(as_cstring(&type->name)) + "." + name;

    // Standard search.
    Term* result = find_method_with_search_name(block, type, searchName.c_str());

    if (result != NULL)
        return result;

    // If the type name is complex (such as List<int>), then try searching
    // for the base type name (such as List).
    std::string baseTypeSymbol = get_base_type_name(as_cstring(&type->name));
    if (baseTypeSymbol != "") {
        std::string searchName = baseTypeSymbol + "." + name;
        result = find_method_with_search_name(block, type, searchName.c_str());

        if (result != NULL)
            return result;
    }

    return NULL;
}

void install_type(Term* term, Type* type)
{
    // Type* oldType = as_type(term);
    set_type(term_value(term), type);
}

Type* find_type(Block* block, const char* name)
{
    Term* term = block->get(name);
    if (term == NULL)
        return NULL;
    if (!is_type(term))
        return NULL;
    return as_type(term);
}

void set_type_list(caValue* value, Type* type1)
{
    set_list(value, 1);
    set_type(list_get(value,0), type1);
}

void set_type_list(caValue* value, Type* type1, Type* type2)
{
    set_list(value, 2);
    set_type(list_get(value,0), type1);
    set_type(list_get(value,1), type2);
}
void set_type_list(caValue* value, Type* type1, Type* type2, Type* type3)
{
    set_list(value, 3);
    set_type(list_get(value,0), type1);
    set_type(list_get(value,1), type2);
    set_type(list_get(value,2), type3);
}

} // namespace circa

void circa_setup_int_type(caType* type)
{
    ca_assert(!type->inUse);
    circa::int_t::setup_type(type);
}

void circa_setup_pointer_type(caType* type)
{
    ca_assert(!type->inUse);
    circa::opaque_pointer_t::setup_type(type);
}
