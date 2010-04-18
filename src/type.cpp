// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {

namespace type_t {

    void decref(Type* type)
    {
        if (type->permanent)
            return;
        assert(type->refCount > 0);
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
        assert(is_type(value));
        decref((Type*) get_pointer(value));
    }
    void copy(TaggedValue* source, TaggedValue* dest)
    {
        assert(is_type(source));
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
            assert(field != NULL);
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

    void name_accessor(EvalContext*, Term* caller)
    {
        set_str(caller, as_type(caller->input(0)).name);
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
    Branch& get_attributes(Term* type)
    {
        return as_type(type).attributes;
    }
    Branch& get_member_functions(Term* type)
    {
        return as_type(type).memberFunctions;
    }
    Term* get_default_value(Term* type)
    {
        Branch& attributes = as_type(type).attributes;
        if (attributes.length() < 1) return NULL;
        return attributes[0];
    }
    void enable_default_value(Term* type)
    {
        if (get_default_value(type) == NULL)
            create_value(type_t::get_attributes(type), VOID_TYPE, "defaultValue");
        change_type(get_default_value(type), type);
    }

} // namespace type_t

bool is_native_type(Term* type)
{
    return !is_branch_based_type(type);
}

Type* declared_type(Term* term)
{
    // Today this is a handy way to avoid writing &as_type
    // In the future, Term->type will be going away, so use this function.
    return &as_type(term->type);
}

Type& as_type(Term *term)
{
    assert(get_type_value(term) != NULL);

    // don't use assert_type here because assert_type uses as_type
    assert(term->type == TYPE_TYPE);

    return *get_type_value(term);
}

bool matches_type(Type* type, Term* term)
{
    // Don't check if term outputs ANY. This should trigger a runtime check.
    if (term->type == ANY_TYPE)
        return true;

    // Return true if types are the same
    if (&as_type(term->type) == type)
        return true;

    Type::MatchesType matchesType = type->matchesType;

    if (matchesType != NULL)
        return matchesType(type, term);

    // Default behavior, if the above checks didn't pass then return false.
    return false;
}

Term* find_common_type(RefList const& list)
{
    if (list.length() == 0)
        return ANY_TYPE;

    bool all_equal = true;
    for (int i=1; i < list.length(); i++) {
        if (list[0] != list[i]) {
            all_equal = false;
            break;
        }
    }

    if (all_equal)
        return list[0];

    // Special case, allow ints to go into floats
    bool all_are_ints_or_floats = true;
    for (int i=0; i < list.length(); i++) {
        if ((list[i] != INT_TYPE) && (list[i] != FLOAT_TYPE)) {
            all_are_ints_or_floats = false;
            break;
        }
    }

    if (all_are_ints_or_floats)
        return FLOAT_TYPE;

    // Another special case, if all types are branch based then use BRANCH_TYPE
    bool all_are_compound = true;
    for (int i=0; i < list.length(); i++)
        if (!is_branch_based_type(list[i]))
            all_are_compound = false;

    if (all_are_compound)
        return BRANCH_TYPE;

    // Otherwise give up
    return ANY_TYPE;
}

void reset_type(Type* type)
{
    type->remapPointers = NULL;
    type->toString = NULL;
    type->checkInvariants = NULL;
    type->valueFitsType = NULL;
    type->initialize = NULL;
    type->release = NULL;
    type->equals = NULL;
    type->cast = NULL;
}

void initialize_simple_pointer_type(Type* type)
{
    reset_type(type);
}

void assign_value_to_default(Term* term)
{
    if (is_int(term))
        set_int(term, 0);
    else if (is_float(term))
        set_float(term, 0);
    else if (is_string(term))
        set_str(term, "");
    else if (is_bool(term))
        set_bool(term, false);
    else if (is_ref(term))
        set_ref(term, NULL);
    else {

        // check if this type has a default value defined
        Term* defaultValue = type_t::get_default_value(term->type);
        if (defaultValue != NULL && defaultValue->type != VOID_TYPE) {
            copy(defaultValue, term);
            return;
        }

        // Otherwise, if it's branched-based, use the prototype
        if (is_branch(term)) {

            Branch& prototype = type_t::get_prototype(term->type);
            branch_t::branch_copy(prototype, as_branch(term));
            return;
        }
    }
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
