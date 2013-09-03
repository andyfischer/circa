// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "control_flow.h"
#include "kernel.h"
#include "function.h"
#include "hashtable.h"
#include "if_block.h"
#include "inspection.h"
#include "interpreter.h"
#include "parser.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "string_type.h"
#include "names.h"
#include "term.h"
#include "type.h"

namespace circa {

bool is_declared_state(Term* term)
{
    return term->function == FUNCS.declared_state;
}


static bool term_has_state_input(Term* term)
{
    for (int i=0; i < term->numInputs(); i++) {
        if (term_get_bool_input_prop(term, i, "state", false))
            return true;
    }
    return false;
}

void list_visible_declared_state(Block* block, TermList* output)
{
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (is_declared_state(term))
            output->append(term);
    }

    if (is_minor_block(block) && get_parent_block(block) != NULL)
        list_visible_declared_state(get_parent_block(block), output);
}

void declared_state_format_source(caValue* source, Term* term)
{
    if (!term->boolProp("syntax:stateKeyword", false))
        return format_term_source_default_formatting(source, term);

    append_phrase(source, "state ", term, tok_State);

    if (term->hasProperty("syntax:explicitType")) {
        append_phrase(source, term->stringProp("syntax:explicitType",""),
                term, sym_TypeName);
        append_phrase(source, " ", term, tok_Whitespace);
    }

    append_phrase(source, term->name.c_str(), term, sym_TermName);

    Term* defaultValue = NULL;
    Block* initializer = NULL;

    Term* initializerInput = term->input(1);

    if (initializerInput != NULL)
        initializer = initializerInput->nestedContents;

    if (initializer != NULL) {
        defaultValue = initializer->getFromEnd(0)->input(0);
        if (defaultValue->boolProp("hidden", false))
            defaultValue = defaultValue->input(0);
    }

    if (defaultValue != NULL) {
        append_phrase(source, " = ", term, sym_None);
        if (defaultValue->name != "")
            append_phrase(source, get_relative_name_at(term, defaultValue),
                    term, sym_TermName);
        else
            format_term_source(source, defaultValue);
    }
}

} // namespace circa
