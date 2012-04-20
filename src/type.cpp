// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "function.h"
#include "importing_macros.h"
#include "kernel.h"
#include "introspection.h"
#include "parser.h"
#include "source_repro.h"
#include "static_checking.h"
#include "names.h"
#include "tagged_value.h"
#include "term.h"
#include "token.h"
#include "type.h"

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

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "type ", term, phrase_type::KEYWORD);
        append_phrase(source, term->name, term, phrase_type::TYPE_NAME);

        if (as_type(term)->nocopy) {
            append_phrase(source, " ", term, phrase_type::WHITESPACE);
            append_phrase(source, ":nocopy", term, phrase_type::UNDEFINED);
        }

        if (term->boolPropOptional("handle", false)) {
            append_phrase(source, " ", term, phrase_type::WHITESPACE);
            append_phrase(source, ":handle", term, phrase_type::UNDEFINED);
        }

        if (term->boolPropOptional("syntax:semicolon", false)) {
            //append_phrase(source, ";", term, phrase_type::UNDEFINED);
            return;
        }

        append_phrase(source, term->stringPropOptional("syntax:preLBracketWhitespace", " "),
                term, TK_WHITESPACE);
        append_phrase(source, "{", term, TK_LBRACKET);
        append_phrase(source, term->stringPropOptional("syntax:postLBracketWhitespace", " "),
                term, TK_WHITESPACE);

        Branch* contents = nested_contents(term);

        for (int i=0; i < contents->length(); i++) {
            Term* field = contents->get(i);
            ca_assert(field != NULL);
            append_phrase(source, field->stringPropOptional("syntax:preWhitespace",""),
                    term, TK_WHITESPACE);
            append_phrase(source, name_to_string(field->type->name),
                    term, phrase_type::TYPE_NAME);
            append_phrase(source, field->stringPropOptional("syntax:postNameWs"," "),
                    term, TK_WHITESPACE);
            append_phrase(source, field->name, term, TK_IDENTIFIER);
            append_phrase(source, field->stringPropOptional("syntax:postWhitespace",""),
                    term, TK_WHITESPACE);
        }
        append_phrase(source, "}", term, TK_RBRACKET);
    }

    std::string toString(caValue* value)
    {
        return std::string("<Type ")+ name_to_string(as_type(value)->name)+">";
    }
    void setup_type(Type* type)
    {
        type->name = name_from_string("Type");
        type->storageType = STORAGE_TYPE_TYPE;
        type->initialize = type_t::initialize;
        type->copy = copy;
        type->gcRelease = type_gc_release;
        type->formatSource = formatSource;
        type->toString = toString;
    }

    Type::RemapPointers& get_remap_pointers_func(Term* type)
    {
        return as_type(type)->remapPointers;
    }

} // namespace type_t

Type::Type() :
    name(0),
    storageType(STORAGE_TYPE_NULL),
    cppTypeInfo(NULL),
    declaringTerm(NULL),
    initialize(NULL),
    release(NULL),
    copy(NULL),
    reset(NULL),
    equals(NULL),
    cast(NULL),
    staticTypeQuery(NULL),
    toString(NULL),
    formatSource(NULL),
    touch(NULL),
    getIndex(NULL),
    setIndex(NULL),
    getField(NULL),
    setField(NULL),
    numElements(NULL),
    checkInvariants(NULL),
    remapPointers(NULL),
    hashFunc(NULL),
    visitHeap(NULL),
    checksum(NULL),
    gcListReferences(NULL),
    gcRelease(NULL),
    parent(NULL),
    nocopy(false)
{
    // Register ourselves. Start out as 'root'.
    gc_register_new_object((CircaObject*) this, &TYPE_T, true);
}

Type::~Type()
{
    gc_on_object_deleted((CircaObject*) this);
}

Type* declared_type(Term* term)
{
    if (term->type == NULL)
        return NULL;
    return term->type;
}

Type* get_output_type(Term* term, int outputIndex)
{
    if (outputIndex == 0)
        return term->type;

    if (term->function == NULL)
        return &ANY_T;

    Function* attrs = as_function(term->function);

    Function::GetOutputType getOutputType = NULL;
    if (attrs != NULL)
        getOutputType = attrs->getOutputType;

    if (getOutputType != NULL)
        return getOutputType(term, outputIndex);

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

void set_type_property(Type* type, const char* name, caValue* value)
{
    copy(value, type->properties.insert(name));
}

Type* create_type()
{
    Type* t = new Type();
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
    if (query->subject && query->subjectType)
        ca_assert(query->subjectType == declared_type(query->subject));

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

    // Always succeed if types are the same.
    if (query->subjectType == query->type)
        return query->succeed();

    // If output term is ANY type then we cannot statically determine.
    if (query->subjectType == &ANY_T)
        return query->unableToDetermine();

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
    type->storageType = STORAGE_TYPE_NULL;
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

void type_initialize_kernel(Branch* kernel)
{
    IMPLICIT_TYPES = create_branch(kernel, "#implicit_types")->owningTerm;
}

Term* create_tuple_type(List* types)
{
    std::stringstream typeName;
    typeName << "Tuple<";
    for (int i=0; i < types->length(); i++) {
        if (i != 0) typeName << ",";
        typeName << name_to_string(as_type(types->get(i))->name);
    }
    typeName << ">";

    Term* result = create_type(nested_contents(IMPLICIT_TYPES), typeName.str());
    list_t::setup_type(unbox_type(result));

    unbox_type(result)->parent = &LIST_T;

    List& parameter = *set_list(&unbox_type(result)->parameter, types->length());

    for (int i=0; i < types->length(); i++) {
        ca_assert(is_type(types->get(i)));
        set_type(parameter[i], as_type(types->get(i)));
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

Term* find_method_with_search_name(Branch* branch, Type* type, std::string const& searchName)
{
    Term* term = find_name(branch, searchName.c_str());
    if (term != NULL && is_function(term))
        return term;

    // If not found, look in the branch where the type was declared.
    Branch* typeDeclarationBranch = NULL;

    if (type->declaringTerm != NULL)
        typeDeclarationBranch = type->declaringTerm->owningBranch;

    if (typeDeclarationBranch != NULL && typeDeclarationBranch != branch) {
        term = find_name(typeDeclarationBranch, searchName.c_str());
        if (term != NULL && is_function(term))
            return term;
    }

    return NULL;
}

Term* find_method(Branch* branch, Type* type, std::string const& name)
{
    if (type->name == 0)
        return NULL;

    std::string searchName = std::string(name_to_string(type->name)) + "." + name;

    Term* result = find_method_with_search_name(branch, type, searchName);

    if (result != NULL)
        return result;

    // If the type name is complex (such as List<int>), then try searching
    // for the base type name (such as List).
    std::string baseTypeName = get_base_type_name(name_to_string(type->name));
    if (baseTypeName != "") {
        result = find_method_with_search_name(branch, type, baseTypeName + "." + name);

        if (result != NULL)
            return result;
    }

    return NULL;
}

Term* parse_type(Branch* branch, std::string const& decl)
{
    return parser::compile(branch, parser::type_decl, decl);
}

void install_type(Term* term, Type* type)
{
    // Type* oldType = as_type(term);
    set_type(term_value(term), type);
}

Type* get_declared_type(Branch* branch, const char* name)
{
    Term* term = branch->get(name);
    if (term == NULL)
        return NULL;
    if (!is_type(term))
        return NULL;
    return as_type(term);
}

void set_type_list(caValue* value, Type* type1)
{
    List* list = set_list(value, 1);
    set_type(list->get(0), type1);
}

void set_type_list(caValue* value, Type* type1, Type* type2)
{
    List* list = set_list(value, 2);
    set_type(list->get(0), type1);
    set_type(list->get(1), type2);
}
void set_type_list(caValue* value, Type* type1, Type* type2, Type* type3)
{
    List* list = set_list(value, 3);
    set_type(list->get(0), type1);
    set_type(list->get(1), type2);
    set_type(list->get(2), type3);
}

} // namespace circa
