// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

namespace circa {

namespace subroutine_t {
    std::string to_string(Term* term);
    void format_source(StyledSource* source, Term* term);
}

CA_FUNCTION(evaluate_subroutine);

bool is_subroutine(Term* term);
Term* find_enclosing_subroutine(Term* term);

// Perform various steps to finish creating a subroutine
void update_subroutine_return_contents(Term* sub, Term* returnCall);
void initialize_subroutine(Term* sub);
void finish_building_subroutine(Term* sub, Term* outputType);

void subroutine_update_state_type_from_contents(Term* sub);
void subroutine_change_state_type(Term* func, Term* type);

void store_locals(Branch& branch, TaggedValue* storage);
void restore_locals(TaggedValue* storageTv, Branch& branch);

void call_subroutine(Branch& sub, TaggedValue* inputs, TaggedValue* output, TaggedValue* error);
void call_subroutine(Term* sub, TaggedValue* inputs, TaggedValue* output, TaggedValue* error);


} // namespace circa
