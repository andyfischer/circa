// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_FUNCTION_INCLUDED
#define CIRCA_FUNCTION_INCLUDED

#include "common_headers.h"

#include "builtin_types.h" // for List
#include "branch.h"
#include "term.h"

namespace circa {

struct FunctionAttrs
{
    std::string name;
    Ref hiddenStateType;
    bool variableArgs;
    std::string exposedNamePath;
    Ref feedbackFunc;
    std::string description;
    TaggedValue parameter;

    // Functions
    EvaluateFunc evaluate;
    SpecializeTypeFunc specializeType;
    FormatSource formatSource;
    CheckInvariants checkInvariants;

    List parameters;

    FunctionAttrs()
      : variableArgs(false),
        evaluate(NULL),
        specializeType(NULL),
        formatSource(NULL),
        checkInvariants(NULL)
    {}
};

namespace function_attrs_t {
    void initialize(Type* type, TaggedValue* value);
    void release(TaggedValue* value);
    void copy(TaggedValue* source, TaggedValue* dest);
}

namespace function_t {
    FunctionAttrs& get_attrs(Term* function);

    std::string to_string(Term* term);
    std::string get_header_source(Term* term);
    void format_header_source(StyledSource* source, Term* term);
    std::string get_documentation(Term* term);

    bool check_invariants(Term* term, std::string* failureMessage);

    // accessors
    std::string const& get_name(Term* function);
    void set_name(Term* function, std::string const& name);
    Term* get_output_type(Term* function);
    Ref& get_hidden_state_type(Term* function);
    bool get_variable_args(Term* function);
    void set_variable_args(Term* function, bool value);
    Term* get_input_placeholder(Term* function, int index);
    Term* get_input_type(Term* function, int index);
    std::string const& get_input_name(Term* function, int index);
    bool get_input_modified(Term* function, int index);
    bool get_input_meta(Term* function, int index);
    void set_input_meta(Term* function, int index, bool value);
    std::string const& get_exposed_name_path(Term* function);
    void set_exposed_name_path(Term* func, std::string const& value);
    Ref& get_feedback_func(Term* function);
    TaggedValue* get_parameters(Term* function);
    std::string const& get_description(Term* function);

    EvaluateFunc& get_evaluate(Term* function);
    SpecializeTypeFunc& get_specialize_type(Term* function);

    int num_inputs(Term* function);
}

bool is_function(Term* term);
bool is_function_attrs(Term* term);
FunctionAttrs& as_function_attrs(Term* term);

std::string get_placeholder_name_for_index(int index);

void initialize_function_prototype(Branch& contents);

bool is_callable(Term* term);
bool inputs_fit_function(Term* function, RefList const& inputs);
bool inputs_fit_function_dynamic(Term* func, RefList const& inputs);

Term* create_overloaded_function(Branch& branch, std::string const& name,
        RefList const& overloads=RefList());
Term* function_get_specialized_output_type(Term* function, Term* call);
void function_set_use_input_as_output(Term* function, int index, bool value);

bool is_native_function(Term* function);

// If function is overloaded, this returns an overload which is appropriate for 'inputs'.
// May return UNKNOWN_FUNCTION if none is found.
Term* specialize_function(Term* function, RefList const& inputs);

} // namespace circa

#endif
