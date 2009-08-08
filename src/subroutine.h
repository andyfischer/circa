// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef CIRCA_SUBROUTINE_INCLUDED
#define CIRCA_SUBROUTINE_INCLUDED

namespace circa {

namespace subroutine_t {
    std::string to_string(Term* term);
}

bool is_subroutine(Term* term);
Branch& call_subroutine(Branch& branch, std::string const& functionName);
void subroutine_update_hidden_state_type(Term* sub);
void subroutine_call_evaluate(Term* caller);
bool is_subroutine_state_expanded(Term* term);
void expand_subroutines_hidden_state(Term* call, Term* state);

} // namespace circa

#endif // CIRCA_SUBROUTINE_INCLUDED
