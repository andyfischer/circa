// Copyright 2008 Andrew Fischer

#ifndef CIRCA_FUNCTION_INCLUDED
#define CIRCA_FUNCTION_INCLUDED

#include "common_headers.h"

#include "term.h"
#include "branch.h"

namespace circa {

#define INPUT_PLACEHOLDER_PREFIX "#input-"
#define OUTPUT_PLACEHOLDER_NAME "#out"

namespace function_t {
    void assign(Term* source, Term* dest);
    void remapPointers(Term* term, ReferenceMap const& map);
    std::string to_string(Term* term);
}

bool is_function(Term* term);

std::string get_placeholder_name_for_index(int index);

void initialize_function_data(Term* term);

std::string& function_get_name(Term* function);
Ref& function_get_output_type(Term* function);
Ref& function_get_hidden_state_type(Term* function);
bool& function_get_variable_args(Term* function);
Term*& function_get_input_placeholder(Term* function, int index);
Term* function_get_input_type(Term* function, int index);
std::string const& function_get_input_name(Term* function, int index);
bool function_get_input_modified(Term* function, int index);
bool function_get_input_meta(Term* function, int index);
void function_set_input_meta(Term* function, int index, bool value);
Ref& function_get_feedback_func(Term* function);
Branch& function_get_parameters(Term* func);

EvaluateFunc& function_get_evaluate(Term* function);
SpecializeTypeFunc& function_get_specialize_type(Term* function);
ToSourceStringFunc& function_get_to_source_string(Term* function);

int function_num_inputs(Term* function);

bool is_callable(Term* term);
bool inputs_fit_function(Term* func, RefList const& inputs);
Term* create_overloaded_function(Branch* branch, std::string const& name);
Term* specialize_function(Term* func, RefList const& inputs);

} // namespace circa

#endif
