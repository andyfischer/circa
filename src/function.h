// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef CIRCA_FUNCTION_INCLUDED
#define CIRCA_FUNCTION_INCLUDED

#include "common_headers.h"

#include "term.h"
#include "branch.h"

namespace circa {

#define INPUT_PLACEHOLDER_PREFIX "#input_"

namespace function_t {
    std::string to_string(Term* term);
    bool check_invariants(Term* term, std::string* failureMessage);

    // accessors
    std::string& get_name(Term* function);
    Term* get_output_type(Term* function);
    Ref& get_hidden_state_type(Term* function);
    bool& get_variable_args(Term* function);
    Term* get_input_placeholder(Term* function, int index);
    Term* get_input_type(Term* function, int index);
    std::string const& get_input_name(Term* function, int index);
    bool get_input_modified(Term* function, int index);
    bool get_input_meta(Term* function, int index);
    void set_input_meta(Term* function, int index, bool value);
    Ref& get_feedback_func(Term* function);
    Branch& get_parameters(Term* func);

    EvaluateFunc& get_evaluate(Term* function);
    SpecializeTypeFunc& get_specialize_type(Term* function);
    ToSourceStringFunc& get_to_source_string(Term* function);

    int num_inputs(Term* function);
}

bool is_function(Term* term);

std::string get_placeholder_name_for_index(int index);

void initialize_function_data(Term* term);

bool is_callable(Term* term);
bool inputs_fit_function(Term* func, RefList const& inputs);
Term* create_overloaded_function(Branch& branch, std::string const& name);

// If func is overloaded, this returns an overload which is appropriate for 'inputs'.
// May return UNKNOWN_FUNCTION if none is found.
Term* specialize_function(Term* func, RefList const& inputs);

} // namespace circa

#endif
