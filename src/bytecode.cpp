// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "blob.h"
#include "bytecode.h"
#include "stateful_code.h"
#include "tagged_value.h"
#include "term_list.h"

namespace circa {

void bc_pack_state(caValue* bytecode, int termIndex, int frameDepth)
{
    blob_append_char(bytecode, bc_PackState);
    blob_append_int(bytecode, termIndex);
    blob_append_int(bytecode, frameDepth);
}

void bc_write_pack_state_steps(caValue* bytecode, Block* block)
{
    TermList declaredStateTerms;
    list_visible_declared_state(block, &declaredStateTerms);

    for (int i=0; i < declaredStateTerms.length(); i++) {
        Term* declaredState = declaredStateTerms[i];

    }
}

} // namespace circa
