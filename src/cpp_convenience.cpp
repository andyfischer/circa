// Copyright 2009 Paul Hodge

#include "circa.h"

namespace circa {

template <> int& as(Term* term) { return as_int(term); }
template <> float& as(Term* term) { return as_float(term); }
template <> bool& as(Term* term) { return as_bool(term); }
template <> std::string& as(Term* term) { return as_string(term); }

} // namespace circa
