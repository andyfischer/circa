// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {

void initialize_type_prototype(Branch& contents)
{
    // Type is not yet prototype-based, so this function isn't currently used.
    
    /* Type has the following layout:
      {
        [0] #attributes {
          [0]  string name
          [1]  bool isPointer
          [2] List parameters
          [3] void/any defaultValue
          [4] List memberFunctions 
          [5]  std_type_info cppTypeInfo
          [6]  AllocFunc alloc
          [7]  DeallocFunc dealloc
          [8]  AllocFunc initialize
          [9]  EqualsFunc equals
          [10]  RemapPointersFunc remapPointers
          [11]  ToStringFunc toString
          [12]  CheckInvariantsFunc checkInvariants
        }
        [1..n-1] prototype
      }
    */

    Term* attributesTerm = create_value(contents, BRANCH_TYPE, "#attributes");
    set_source_hidden(attributesTerm, false);
    Branch& attributes = as_branch(attributesTerm);
    create_string(attributes, "", "name");
    create_bool(attributes, false, "isPointer");
    create_list(attributes, "parameters");
    create_void(attributes, "defaultValue");
    create_list(attributes, "memberFunctions");
    create_value(attributes, STD_TYPE_INFO_TYPE, "cppTypeInfo");
    create_value(attributes, ALLOC_THUNK_TYPE, "alloc");
    create_value(attributes, DEALLOC_THUNK_TYPE, "dealloc");
    create_value(attributes, ALLOC_THUNK_TYPE, "initialize");
    create_value(attributes, EQUALS_THUNK_TYPE, "equals");
    create_value(attributes, REMAP_POINTERS_THUNK_TYPE, "remapPointers");
    create_value(attributes, TO_STRING_THUNK_TYPE, "toString");
    create_value(attributes, CHECK_INVARIANTS_THUNK_TYPE, "checkInvariants");
}

namespace type_t {
    void alloc(Term* type, Term* term)
    {
        term->value = new Type();
    }
    void dealloc(Term* type, Term* term)
    {
        // FIXME
        // delete reinterpret_cast<Type*>(data);
    }
    std::string to_string(Term* term)
    {
        if (is_native_type(term))
            return "<NativeType " + term->name + ">";

        // Generate source for a Type declaration
        std::stringstream out;

        out << "type " << term->name;
        out << term->stringPropOptional("syntaxHints:preLBracketWhitespace", " ");
        out << "{";
        out << term->stringPropOptional("syntaxHints:postLBracketWhitespace", " ");

        Branch& prototype = type_t::get_prototype(term);

        for (int i=0; i < prototype.length(); i++) {
            Term* field = prototype[i];
            assert(field != NULL);
            out << field->stringPropOptional("syntaxHints:preWhitespace","");
            out << field->type->name;
            out << field->stringPropOptional("syntaxHints:postNameWs"," ");
            out << field->name;
            out << field->stringPropOptional("syntaxHints:postWhitespace","");
        }
        out << "}";

        return out.str();
    }

    void assign(Term* source, Term* dest)
    {
        Type* sourceValue = (Type*) source->value;
        Type* destValue = (Type*) dest->value;

        if (sourceValue == destValue)
            return;

        dest->value = sourceValue;
    }

    void remap_pointers(Term *type, ReferenceMap const& map)
    {
        Branch& prototype = type_t::get_prototype(type);

        for (int field_i=0; field_i < prototype.length(); field_i++)
            prototype.set(field_i, map.getRemapped(prototype[field_i]));
    }

    void name_accessor(Term* caller)
    {
        as_string(caller) = type_t::get_name(caller->input(0));
    }

    std::string& get_name(Term* type)
    {
        return as_type(type).name;
    }
    bool& get_is_pointer(Term* type)
    {
        return as_type(type).isPointer;
    }
    const std::type_info*& get_std_type_info(Term* type)
    {
        return as_type(type).cppTypeInfo;
    }
    AllocFunc& get_alloc_func(Term* type)
    {
        return as_type(type).alloc;
    }
    DeallocFunc& get_dealloc_func(Term* type)
    {
        return as_type(type).dealloc;
    }
    AllocFunc& get_initialize_func(Term* type)
    {
        return as_type(type).initialize;
    }
    AssignFunc& get_assign_func(Term* type)
    {
        return as_type(type).assign;
    }
    EqualsFunc& get_equals_func(Term* type)
    {
        return as_type(type).equals;
    }
    RemapPointersFunc& get_remap_pointers_func(Term* type)
    {
        return as_type(type).remapPointers;
    }
    ToStringFunc& get_to_string_func(Term* type)
    {
        return as_type(type).toString;
    }
    CheckInvariantsFunc& get_check_invariants_func(Term* type)
    {
        return as_type(type).checkInvariants;
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
    int find_field_index(Term* type, std::string const& name)
    {
        return type_t::get_prototype(type).findIndex(name);
    }
    void enable_default_value(Term* type)
    {
        if (get_default_value(type) == NULL)
            create_value(type_t::get_attributes(type), VOID_TYPE, "defaultValue");
        change_type(get_default_value(type), type);
        alloc_value(get_default_value(type));
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

    if (!identity_equals(term->type, type))
        return false;

    return true;
}

bool is_type(Term* term)
{
    assert(term != NULL);
    return term->type == TYPE_TYPE;
}

bool is_native_type(Term* type)
{
    return !is_compound_type(type);
}

bool is_compound_type(Term* type)
{
    return type_t::get_alloc_func(type) == branch_t::alloc;
}

Type& as_type(Term *term)
{
    // don't call alloc_value because that calls as_type
    assert(term->value != NULL);

    // don't use assert_type here because assert_type uses as_type
    assert(term->type == TYPE_TYPE);

    return *((Type*) term->value);
}

bool value_fits_type(Term* valueTerm, Term* type, std::string* errorReason)
{
    // Always match if they have the same exact type
    if (identity_equals(valueTerm->type, type))
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
    if (identity_equals(type, LIST_TYPE))
        return true;
    if (identity_equals(type, BRANCH_TYPE))
        return true;

    // Special case hack, also accept any compound type against OverloadedFunction
    // (Need to have a way for a type to specify that it accepts a variable number of
    // items)
    if (identity_equals(type, OVERLOADED_FUNCTION_TYPE))
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

namespace type_private {
    void empty_allocate(Term* type, Term* term) { term->value = NULL; }
    void empty_duplicate_function(Term*,Term*) {}
}

void initialize_empty_type(Term* term)
{
    type_t::get_alloc_func(term) = type_private::empty_allocate;
    type_t::get_assign_func(term) = type_private::empty_duplicate_function;
}

void initialize_compound_type(Term* term)
{
    type_t::get_alloc_func(term) = branch_t::alloc;
    type_t::get_dealloc_func(term) = branch_t::dealloc;
    type_t::get_assign_func(term) = branch_t::assign;
    type_t::get_remap_pointers_func(term) = branch_t::hosted_remap_pointers;
    type_t::get_equals_func(term) = branch_t::equals;
    type_t::get_to_string_func(term) = compound_type_to_string;
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

bool identity_equals(Term* a, Term* b)
{
    return a->value == b->value;
}

bool equals(Term* a, Term* b)
{
    if (a->type != b->type)
        return false;

    EqualsFunc equals_func = type_t::get_equals_func(a->type);

    if (equals_func == NULL)
        throw std::runtime_error("type "+type_t::get_name(a->type)+" has no equals function");

    return equals_func(a,b);
}

std::string to_string(Term* term)
{
    ToStringFunc func = type_t::get_to_string_func(term->type);

    if (func == NULL) {
        // Generic to-string
        std::stringstream result;
        result << "<" << type_t::get_name(term->type) << " 0x";
        result << std::hex << term->value << ">";
        return result.str();
    }
    else if (!is_value_alloced(term))
        return "<NULL>";
    else
        return func(term);
}

void alloc_value(Term* term)
{
    if (is_value_alloced(term)) return;

    AllocFunc alloc = type_t::get_alloc_func(term->type);

    if (alloc == NULL)
        // this happens while bootstrapping
        term->value = NULL;
    else {
        alloc(term->type, term);

        if (term->type != TYPE_TYPE)
            assign_value_to_default(term);

        if (is_branch(term))
            as_branch(term).owningTerm = term;
    }
}

void dealloc_value(Term* term)
{
    if (term->type == NULL) return;
    if (!is_value_alloced(term)) return;

    DeallocFunc dealloc = type_t::get_dealloc_func(term->type);

    if (!is_value_alloced(term->type)) {
        std::cout << "warn: in dealloc_value, type is undefined" << std::endl;
        term->value = NULL;
        return;
    }

    if (dealloc != NULL)
        dealloc(term->type, term);

    term->value = NULL;
}

bool is_value_alloced(Term* term)
{
    if (term->type == NULL) {
        assert(term->value == NULL);
        return false;
    }

    if (!type_t::get_is_pointer(term->type))
        return true;
    else
        return term->value != NULL;
}

void assign_value(Term* source, Term* dest)
{
    if (!is_value_alloced(source)) {
        dealloc_value(dest);
        return;
    }

    // Do a type specialization if dest has type 'any'.
    // This might be removed once type inference rules are smarter.
    if (dest->type == ANY_TYPE)
        specialize_type(dest, source->type);

    if (!value_fits_type(source, dest->type)) {
        std::stringstream err;
        err << "In assign_value, element of type " << source->type->name <<
            " doesn't fit in type " << dest->type->name;
        throw std::runtime_error(err.str());
    }

    if (!is_value_alloced(dest))
        alloc_value(dest);

    AssignFunc assign = type_t::get_assign_func(dest->type);

    if (assign == NULL)
        throw std::runtime_error("type "+type_t::get_name(dest->type)+" has no assign function");

    assign(source, dest);
}

void assign_value_to_default(Term* term)
{
    if (is_int(term))
        as_int(term) = 0;
    else if (is_float(term))
        as_float(term) = 0;
    else if (is_string(term))
        as_string(term) = "";
    else if (is_bool(term))
        as_bool(term) = false;
    else if (is_ref(term))
        as_ref(term) = NULL;
    else {

        // check if this type has a default value defined
        Term* defaultValue = type_t::get_default_value(term->type);
        if (defaultValue != NULL && defaultValue->type != VOID_TYPE) {
            assign_value(defaultValue, term);
            return;
        }

        // Otherwise, if it's branched-based, use the prototype
        if (is_branch(term)) {
            as_branch(term).clear();
            duplicate_branch(type_t::get_prototype(term->type), as_branch(term));
            return;
        }
    }
}

bool check_invariants(Term* term, std::string* failureMessage)
{
    if (type_t::get_check_invariants_func(term->type) == NULL)
        return true;

    return type_t::get_check_invariants_func(term->type)(term, failureMessage);
}

Term* parse_type(Branch& branch, std::string const& decl)
{
    return parser::compile(&branch, parser::type_decl, decl);
}

} // namespace circa
