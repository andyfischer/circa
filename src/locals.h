// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {
    
int get_output_count(Term* term);
void update_locals_index_for_new_term(Term*);

void refresh_locals_indices(Branch&, int startingAt = 0);

} // namespace circa
