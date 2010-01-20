// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_BUILTIN_TYPES_INCLUDED
#define CIRCA_BUILTIN_TYPES_INCLUDED

namespace circa {

extern Type* STRING_TYPE_2;
extern Type* REF_TYPE_2;

namespace int_t {
    void initialize(Type* type, TaggedValue& value);
    std::string to_string(Term* term);
}

namespace float_t {
    void initialize(Type* type, TaggedValue& value);
    void cast(Type* type, TaggedValue* source, TaggedValue* dest);
    bool equals(Term* a, Term* b);
    std::string to_string(Term* term);
}

namespace bool_t {
    std::string to_string(Term* term);
}

namespace set_t {
    void add(Branch& branch, Term* value);
}

namespace list_t {
    void append(Branch& branch, Term* value);
    std::string to_string(Term* caller);
}

namespace dict_t {
    std::string to_string(Branch& branch);
}

namespace string_t {
    void initialize(Type* type, TaggedValue* value);
    void assign(TaggedValue* source, TaggedValue* dest);
    bool equals(TaggedValue* lhs, TaggedValue* rhs);
    std::string to_string(Term* term);
    void length(Term* term);
    void substr(Term* term);
}

namespace ref_t {
    std::string to_string(Term* term);
    void initialize(Type* type, TaggedValue* value);
    void assign(TaggedValue* source, TaggedValue* dest);
    bool equals(Term* lhs, Term* rhs);
    void get_name(Term* caller);
    void hosted_to_string(Term* caller);
    void get_function(Term* caller);
    void hosted_typeof(Term* caller);
    void assign(Term* caller);
    void tweak(Term* caller);
    void asint(Term* caller);
    void asfloat(Term* caller);
    void get_input(Term* caller);
    void num_inputs(Term* caller);
    void get_source_location(Term* caller);
}

namespace type_t {
    void initialize(Type* type, TaggedValue* value);
    void assign(TaggedValue* source, TaggedValue* dest);
}

void initialize_builtin_types();
void setup_builtin_types(Branch& kernel);
void parse_builtin_types(Branch& kernel);

}

#endif
