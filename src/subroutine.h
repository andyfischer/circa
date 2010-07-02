// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

namespace circa {

namespace subroutine_t {
    std::string to_string(Term* term);
    void format_source(StyledSource* source, Term* term);
    void evaluate(EvalContext*, Term* caller, Term*, RefList const& inputs, TaggedValue*);
}

bool is_subroutine(Term* term);

// Perform various steps to finish creating a subroutine
void finish_building_subroutine(Term* sub, Term* outputType);

void subroutine_update_hidden_state_type(Term* sub);
bool is_subroutine_state_expanded(Term* term);
void expand_subroutines_hidden_state(Term* call, Term* state);

} // namespace circa
