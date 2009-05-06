// Copyright 2009 Paul Hodge

#ifndef CIRCA_SUBROUTINE_INCLUDED
#define CIRCA_SUBROUTINE_INCLUDED

namespace circa {

bool is_subroutine(Term* term);
void initialize_as_subroutine(Function& func);
void initialize_subroutine_call(Term* term);
Branch& call_subroutine(Branch& branch, std::string const& functionName);
Branch& get_subroutine_branch(Term* term);

} // namespace circa

#endif // CIRCA_SUBROUTINE_INCLUDED
