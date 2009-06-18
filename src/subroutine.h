// Copyright 2009 Paul Hodge

#ifndef CIRCA_SUBROUTINE_INCLUDED
#define CIRCA_SUBROUTINE_INCLUDED

namespace circa {

namespace subroutine_t {
    std::string to_string(Term* term);
}

bool is_subroutine(Term* term);
Function& get_subroutines_function_def(Term* term);
void initialize_subroutine(Term* term);
Branch& call_subroutine(Branch& branch, std::string const& functionName);
void subroutine_update_hidden_state_type(Term* sub);
void subroutine_call_evaluate(Term* caller);
bool is_subroutine_state_expanded(Term* term);
void expand_subroutines_hidden_state(Term* call, Term* state);
bool sanity_check_subroutine(Term* sub, std::string& message);

} // namespace circa

#endif // CIRCA_SUBROUTINE_INCLUDED
