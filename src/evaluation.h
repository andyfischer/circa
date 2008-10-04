#ifndef CIRCA__EVALUATOR__INCLUDED
#define CIRCA__EVALUATOR__INCLUDED

#include "common_headers.h"

namespace circa {

void evaluate_term(Term* term);
void evaluate_branch(Branch& branch);
Branch* evaluate_file(std::string const& filename);

} // namespace circa

#endif
