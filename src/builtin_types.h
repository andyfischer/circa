// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"
#include "tagged_value.h"

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
    void add(List* list, TaggedValue* value);
    void setup_type(Type* type);
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
    bool matches_type(Type* type, Type* otherType);
    void cast(Type* type, TaggedValue* source, TaggedValue* dest);
    void cast2(CastResult* result, TaggedValue* source, Type* type,
        TaggedValue* dest, bool checkOnly);
}

namespace branch_ref_t {
    void set_from_ref(TaggedValue* value, Term* ref);
    void setup_type(Type* type);
    Branch& get_target_branch(TaggedValue* value);
}

namespace void_t { void setup_type(Type* type); }
namespace styled_source_t { void setup_type(Type* type); }
namespace indexable_t { void setup_type(Type* type); }
namespace callable_t { void setup_type(Type* type); }

namespace point_t {

    // Helper functions:
    void read(TaggedValue* value, float* x, float* y);
    void write(TaggedValue* value, float x, float y);
}

void initialize_primitive_types(Branch& kernel);

void setup_types(Branch& kernel);
void parse_types(Branch& kernel);
void post_setup_types();

}
