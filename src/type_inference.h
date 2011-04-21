// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// Returns a common type, which is guaranteed to hold all the types in this
// list. Currently, this is not very sophisticated.
Term* find_common_type(TermList const& list);

// Guesses at the declared type for a get_index call on this term. Currently
// this is not very sophisticated, since we don't have generic types.
Term* find_type_of_get_index(Term* listTerm);

}
