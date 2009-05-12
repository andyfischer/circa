// Copyright 2009 Andrew Fischer

#ifndef CIRCA_SUBROUTINE_INCLUDED
#define CIRCA_SUBROUTINE_INCLUDED

namespace circa {

bool is_subroutine(Term* term);
Function& get_subroutines_function_def(Term* term);
void register_subroutine_type(Branch& kernel);
void initialize_subroutine(Term* term);
//void initialize_as_subroutine(Function& func);
void initialize_subroutine_state(Term* term, Branch& state);
Branch& call_subroutine(Branch& branch, std::string const& functionName);
Branch& get_state_for_subroutine_call(Term* term);

} // namespace circa

#endif // CIRCA_SUBROUTINE_INCLUDED
