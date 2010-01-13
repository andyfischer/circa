// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

template <> int& as(Term* term) { throw std::runtime_error("as<int> disabled"); }
template <> float& as(Term* term) { throw std::runtime_error("as<float> disabled"); }
template <> bool& as(Term* term) { throw std::runtime_error("as<bool> disabled"); }
template <> std::string& as(Term* term) { throw std::runtime_error("as<string> disabled"); }

} // namespace circa
