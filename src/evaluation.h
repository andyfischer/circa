#ifndef CIRCA_EVALUATOR_INCLUDED
#define CIRCA_EVALUATOR_INCLUDED

#include "common_headers.h"

namespace circa {

void evaluate_term(Term* term);
void evaluate_branch(Branch& branch);
Branch* evaluate_file(std::string const& filename);

} // namespace circa

#endif
