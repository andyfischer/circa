// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef CIRCA_SUBROUTINE_INCLUDED
#define CIRCA_SUBROUTINE_INCLUDED

namespace circa {

namespace subroutine_t {
    std::string to_string(Term* term);
    void evaluate(Term* caller);
}

bool is_subroutine(Term* term);

// Perform various steps to finish creating a subroutine
void finish_building_subroutine(Term* sub, Term* outputType);

void subroutine_update_hidden_state_type(Term* sub);
bool is_subroutine_state_expanded(Term* term);
void expand_subroutines_hidden_state(Term* call, Term* state);

} // namespace circa

#endif // CIRCA_SUBROUTINE_INCLUDED
