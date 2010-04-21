// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_BUILTIN_TYPES_INCLUDED
#define CIRCA_BUILTIN_TYPES_INCLUDED

#include "common_headers.h"

namespace circa {

namespace int_t {
    void setup_type(Type* type);
}

namespace float_t {
    bool equals(TaggedValue* a, TaggedValue* b);
    void setup_type(Type* type);
}

namespace bool_t {
    void setup_type(Type* type);
}

namespace set_t {
    void add(Branch& branch, Term* value);
    void setup_type(Type* type);
}

namespace old_list_t {
    void setup(Type*);
}

namespace list_t {
    bool is_list(TaggedValue*);
}

namespace dict_t {
    std::string to_string(Branch& branch);
}

namespace string_t {
    void setup_type(Type* type);
    void postponed_setup_type(Type* type);
}

namespace ref_t {
    void setup_type(Type* type);
    void postponed_setup_type(Type* type);
}

namespace any_t {
    std::string to_string(TaggedValue*);
    bool matches_type(Type* type, Term* term);
}

namespace branch_ref_t {
    void setup_type(Type* type);
    Branch& get_target_branch(TaggedValue* value);
}

namespace void_t { void setup_type(Type* type); }
namespace styled_source_t { void setup_type(Type* type); }
namespace indexable_t { void setup_type(Type* type); }
namespace callable_t { void setup_type(Type* type); }

namespace point_t {

    // Helper functions:
    void read(Term* term, float* x, float* y);
    void write(Term* term, float x, float y);
}

void initialize_primitive_types(Branch& kernel);

// Do some more setup, after all the standard builtin types have been created.
void post_setup_primitive_types();

void setup_builtin_types(Branch& kernel);
void parse_builtin_types(Branch& kernel);

}

#endif
