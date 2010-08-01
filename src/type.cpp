// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"
#include "debug_valid_objects.h"
#include "importing_macros.h"

namespace circa {

Term* IMPLICIT_TYPES = NULL;

namespace type_t {

    void decref(Type* type)
    {
        if (type->permanent)
            return;
        ca_assert(type->refCount > 0);
        type->refCount--;
        if (type->refCount == 0)
            delete type;
    }

    void initialize(Type*, TaggedValue* value)
    {
        Type* type = Type::create();
        type->refCount++;
        set_pointer(value, type);
    }
    void release(TaggedValue* value)
    {
        ca_assert(is_type(value));
        decref((Type*) get_pointer(value));
    }
    void copy(TaggedValue* source, TaggedValue* dest)
    {
        ca_assert(is_type(source));
        copy((Type*) get_pointer(source), dest);
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "type ", term, phrase_type::KEYWORD);
        append_phrase(source, term->name, term, phrase_type::TYPE_NAME);
        append_phrase(source, term->stringPropOptional("syntax:preLBracketWhitespace", " "),
                term, token::WHITESPACE);
        append_phrase(source, "{", term, token::LBRACKET);
        append_phrase(source, term->stringPropOptional("syntax:postLBracketWhitespace", " "),
                term, token::WHITESPACE);

        Branch& prototype = type_t::get_prototype(term);

        for (int i=0; i < prototype.length(); i++) {
            Term* field = prototype[i];
            ca_assert(field != NULL);
            append_phrase(source, field->stringPropOptional("syntax:preWhitespace",""),
                    term, token::WHITESPACE);
            append_phrase(source, field->type->name, term, phrase_type::TYPE_NAME);
            append_phrase(source, field->stringPropOptional("syntax:postNameWs"," "),
                    term, token::WHITESPACE);
            append_phrase(source, field->name, term, token::IDENTIFIER);
            append_phrase(source, field->stringPropOptional("syntax:postWhitespace",""),
                    term, token::WHITESPACE);
        }
        append_phrase(source, "}", term, token::RBRACKET);
    }

    void remap_pointers(Term *type, ReferenceMap const& map)
    {
        Branch& prototype = type_t::get_prototype(type);

        for (int field_i=0; field_i < prototype.length(); field_i++) {
            Term* orig = prototype[field_i];
            Term* remapped = map.getRemapped(orig);
            assert_valid_term(orig);
            assert_valid_term(remapped);
            prototype.set(field_i, remapped);
        }
    }

    CA_FUNCTION(name_accessor)
    {
        set_str(OUTPUT, as_type(INPUT(0)).name);
    }

    void setup_type(Term* type)
    {
        import_member_function(type, name_accessor, "name(Type) -> string");
    }

    void copy(Type* value, TaggedValue* dest)
    {
        Type* oldType = (Type*) get_pointer(dest);

        if (value == oldType)
            return;

        set_pointer(dest, value);
        value->refCount++;
        decref(oldType);
    }

    Type::RemapPointers& get_remap_pointers_func(Term* type)
    {
        return as_type(type).remapPointers;
    }
    Branch& get_prototype(Term* type)
    {
        return as_type(type).prototype;
    }
    Branch& get_prototype(Type* type)
    {
        return type->prototype;
    }
    Branch& get_attributes(Term* type)
    {
        return as_type(type).attributes;
    }
    TaggedValue* get_default_value(Type* type)
    {
        return &type->defaultValue;
    }

} // namespace type_t

bool is_native_type(Term* type)
{
    return !is_branch_based_type(type);
}

Type* declared_type(Term* term)
{
    return &as_type(term->type);
}

Type& as_type(Term *term)
{
    ca_assert(get_type_value(term) != NULL);

    // don't use ca_assert_type here because ca_assert_type uses as_type
    ca_assert(term->type == TYPE_TYPE);

    return *get_type_value(term);
}

Type* type_contents(Term* type)
{
    return &as_type(type);
}

bool value_fits_type(TaggedValue* value, Type* type)
{
    if (is_subtype(type, value->value_type))
        return true;

    Type::ValueFitsType func = type->valueFitsType;
    if (func != NULL)
        return func(type, value);

    return false;
}

static void run_static_type_query(Type* type, Term* outputTerm, StaticTypeQuery* result)
{
    // Always succeed if types are the same
    if (declared_type(outputTerm) == type)
        return result->succeed();

    // If output term is ANY type then we cannot statically determine.
    if (outputTerm->type == ANY_TYPE)
        return result->unableToDetermine();

    // Try using the type's static query func
    Type::StaticTypeQueryFunc staticTypeQueryFunc = type->staticTypeQuery;
    if (staticTypeQueryFunc != NULL) {
        result->targetTerm = outputTerm;
        staticTypeQueryFunc(type, result);
        return;
    }

    // Finally, use is_subtype
    if (is_subtype(type, type_contents(outputTerm->type)))
        return result->succeed();
    else
        return result->fail();
}

bool term_output_always_satisfies_type(Term* term, Type* type)
{
    StaticTypeQuery obj;
    run_static_type_query(type, term, &obj);
    return obj.result == StaticTypeQuery::SUCCEED;
}

bool term_output_never_satisfies_type(Term* term, Type* type)
{
    StaticTypeQuery obj;
    run_static_type_query(type, term, &obj);
    return obj.result == StaticTypeQuery::FAIL;
}

bool is_subtype(Type* type, Type* subType)
{
    if (type == subType)
        return true;

    Type::IsSubtype isSubtype = type->isSubtype;
    if (isSubtype == NULL)
        return false;

    return isSubtype(type, subType);
}

void reset_type(Type* type)
{
    type->initialize = NULL;
    type->release = NULL;
    type->copy = NULL;
    type->reset = NULL;
    type->equals = NULL;
    type->cast = NULL;
    type->remapPointers = NULL;
    type->toString = NULL;
    type->formatSource = NULL;
    type->checkInvariants = NULL;
    type->valueFitsType = NULL;
    type->isSubtype = NULL;
    type->staticTypeQuery = NULL;
    type->touch = NULL;
    type->getIndex = NULL;
    type->setIndex = NULL;
    type->getField = NULL;
    type->setField = NULL;
    type->numElements = NULL;
}

void initialize_simple_pointer_type(Type* type)
{
    reset_type(type);
}

void type_initialize_kernel(Branch& kernel)
{
    IMPLICIT_TYPES = create_branch(kernel, "#implicit_types").owningTerm;
}

Term* create_implicit_tuple_type(RefList const& types)
{
    std::stringstream typeName;
    typeName << "Tuple<";
    for (int i=0; i < types.length(); i++) {
        if (i != 0) typeName << ",";
        typeName << type_contents(types[i])->name;
    }
    typeName << ">";

    Term* result = create_type(IMPLICIT_TYPES->nestedContents, typeName.str());
    list_t::setup_type(type_contents(result));
    Branch& prototype = type_contents(result)->prototype;
    type_contents(result)->parent = type_contents(LIST_TYPE);

    for (int i=0; i < types.length(); i++) {
        ca_assert(is_type(types[i]));
        create_value(prototype, types[i]);
    }
    
    return result;
}

Term* find_member_function(Type* type, std::string const& name)
{
    if (type->memberFunctions.contains(name))
        return type->memberFunctions[name];

    if (type->parent != NULL)
        return find_member_function(type->parent, name);

    return NULL;
}

Term* parse_type(Branch& branch, std::string const& decl)
{
    return parser::compile(&branch, parser::type_decl, decl);
}

void
TypeRef::set(Type* target)
{
    if (t == target)
        return;

    Type* previousTarget = t;

    t = target;

    if (t != NULL)
        t->refCount++;

    if (previousTarget != NULL)
        type_t::decref(previousTarget);
}

} // namespace circa
