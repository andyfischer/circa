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
    void length(EvalContext*, Term* term);
    void substr(EvalContext*, Term* term);
}

namespace ref_t {
    std::string to_string(Term* term);
    void initialize(Type* type, TaggedValue* value);
    void assign(TaggedValue* source, TaggedValue* dest);
    bool equals(Term* lhs, Term* rhs);
    void get_name(EvalContext*, Term* caller);
    void hosted_to_string(EvalContext*, Term* caller);
    void get_function(EvalContext*, Term* caller);
    void hosted_typeof(EvalContext*, Term* caller);
    void assign(EvalContext*, Term* caller);
    void tweak(EvalContext*, Term* caller);
    void asint(EvalContext*, Term* caller);
    void asfloat(EvalContext*, Term* caller);
    void get_input(EvalContext*, Term* caller);
    void num_inputs(EvalContext*, Term* caller);
    void get_source_location(EvalContext*, Term* caller);
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
