// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {

namespace type_t {
    std::string to_string(Term* term)
    {
        if (is_native_type(term))
            return "<NativeType " + term->name + ">";

        // Generate source for a Type declaration
        std::stringstream out;

        out << "type " << term->name;
        out << term->stringPropOptional("syntax:preLBracketWhitespace", " ");
        out << "{";
        out << term->stringPropOptional("syntax:postLBracketWhitespace", " ");

        Branch& prototype = type_t::get_prototype(term);

        for (int i=0; i < prototype.length(); i++) {
            Term* field = prototype[i];
            assert(field != NULL);
            out << field->stringPropOptional("syntax:preWhitespace","");
            out << field->type->name;
            out << field->stringPropOptional("syntax:postNameWs"," ");
            out << field->name;
            out << field->stringPropOptional("syntax:postWhitespace","");
        }
        out << "}";

        return out.str();
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
        set_str(caller, type_t::get_name(caller->input(0)));
    }

    std::string& get_name(Term* type)
    {
        return as_type(type).name;
    }
    const std::type_info*& get_std_type_info(Term* type)
    {
        return as_type(type).cppTypeInfo;
    }
    Type::RemapPointersFunc& get_remap_pointers_func(Term* type)
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

bool type_matches(Term *term, Term *type)
{
    assert(term != NULL);

    // any type matches anything
    if (type == ANY_TYPE)
        return true;

    // Allow for compound types to be considered the same.
    // Later there can be more complicated type checking.

    if (type != NULL && is_compound_type(term->type)
            && is_compound_type(type))
        return true;

    if (term->type != type)
        return false;

    return true;
}

bool is_native_type(Term* type)
{
    return !is_compound_type(type);
}

bool is_compound_type(Term* type)
{
    return as_type(type).initialize == branch_t::initialize;
}

Type& as_type(Term *term)
{
    assert(get_type_value(term) != NULL);

    // don't use assert_type here because assert_type uses as_type
    assert(term->type == TYPE_TYPE);

    return *get_type_value(term);
}

bool value_fits_type(Term* valueTerm, Term* type, std::string* errorReason)
{
    // Always match if they have the same exact type
    if (valueTerm->type == type)
        return true;

    // Everything matches against 'any'
    if (type == ANY_TYPE)
        return true;

    // Also, 'any' matches anything.
    if (valueTerm->type == ANY_TYPE)
        return true;

    // Coercion: ints fit in floats
    if ((valueTerm->type == INT_TYPE) && type == FLOAT_TYPE)
        return true;

    // Otherwise, primitive types must fit exactly.
    // So if this is a primitive type, reject it.
    if (!is_compound_type(type)) {
        if (errorReason != NULL)
            *errorReason = "type is primitive, expected exact match";
        return false;
    }

    // If 'type' is a compound type, make sure value is too
    if (!is_compound_type(valueTerm->type)) {
        if (errorReason != NULL)
            *errorReason = "value is primitive, type is compound";
        return false;
    }

    // Every compound type matches against List or Branch
    if (type == LIST_TYPE)
        return true;
    if (type == BRANCH_TYPE)
        return true;

    // Special case hack, also accept any compound type against OverloadedFunction
    // (Need to have a way for a type to specify that it accepts a variable number of
    // items)
    if (type == OVERLOADED_FUNCTION_TYPE)
        return true;

    Branch& value = as_branch(valueTerm);

    // Check if the # of elements matches
    // TODO: Relax this check for lists
    if (value.length() != type_t::get_prototype(type).length()) {
        if (errorReason != NULL) {
            std::stringstream error;
            error << "value has " << value.length() << " elements, type has "
                << type_t::get_prototype(type).length();
            *errorReason = error.str();
        }
        return false;
    }

    // Check each element
    for (int i=0; i < value.length(); i++) {
        if (!value_fits_type(value[i], type_t::get_prototype(type)[i]->type, errorReason)) {

            if (errorReason != NULL) {
                std::stringstream error;
                error << "element " << i << " did not fit:\n" << *errorReason;
                *errorReason = error.str();
            }

            return false;
        }
    }

    return true;
}

bool is_assign_value_possible(Term* source, Term* dest)
{
    return value_fits_type(source, dest->type);
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
        if (!is_compound_type(list[i]))
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
    type->assign = NULL;
    type->release = NULL;
    type->equals = NULL;
    type->cast = NULL;
}
void initialize_compound_type(Term* term)
{
    Type* type = &as_type(term);

    reset_type(type);
    type->initialize = branch_t::initialize;
    type->release = branch_t::release;
    type->assign = branch_t::assign;
    type->cast = branch_t::cast;
    type->equals = branch_t::equals;
    type->toString = compound_type_to_string;
}

void initialize_simple_pointer_type(Type* type)
{
    reset_type(type);

    // Unnecessary but illustrative:
    type->initialize = NULL;
    type->release = NULL;
    type->assign = NULL;
}

std::string compound_type_to_string(Term* caller)
{
    std::stringstream out;
    out << "[";

    Branch& value = as_branch(caller);

    for (int i=0; i < value.length(); i++) {
        if (i != 0)
            out << ", ";
        out << to_string(value[i]);
    }

    out << "]";
    return out.str();
}

std::string to_string(Term* term)
{
    Type* type = term->value_type;

    if (type == NULL)
        return "<NULL>";

    Type::ToStringFunc func = type->toString;

    if (func != NULL)
        return func(term);

    // Generic to-string
    std::stringstream result;
    result << type->name << "#";
    result << std::hex << get_type_value(term) << ">";
    return result.str();
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
            assign_value(defaultValue, term);
            return;
        }

        // Otherwise, if it's branched-based, use the prototype
        if (is_branch(term)) {

            Branch& prototype = type_t::get_prototype(term->type);
            branch_t::assign_overwriting_types(prototype, as_branch(term));
            return;
        }
    }
}

Term* parse_type(Branch& branch, std::string const& decl)
{
    return parser::compile(&branch, parser::type_decl, decl);
}

} // namespace circa
