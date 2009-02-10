// Copyright 2008 Andrew Fischer

#ifndef CIRCA_MIGRATION_INCLUDED
#define CIRCA_MIGRATION_INCLUDED

#include "common_headers.h"

#include "branch.h"

namespace circa {

void migrate_branch(Branch& replacement, Branch& target);
void reload_branch_from_file(Branch& branch);

} // namespace circa

#endif
