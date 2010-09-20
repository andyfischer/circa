// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace do_once_function {

    CA_FUNCTION(empty_evaluate)
    {
    }

    void write_bytecode(bytecode::WriteContext* context, Term* term)
    {
        ca_assert(term->function->name == "do_once");

        TaggedValue fieldName;
        make_string(&fieldName, get_implicit_state_name(term));
        int name = bytecode::write_push_local_op(context, &fieldName);

        int alreadyRan = context->nextStackIndex++;
        bytecode::write_get_state_field(context, NULL, name, -1, alreadyRan);

        bytecode::BytecodePosition jumpToEnd = context->getPosition();
        bytecode::write_jump_if(context, alreadyRan, 0);

        // fixme: set state to true

        // fixme: support state inside do_once

        bytecode::write_bytecode_for_branch(context, term->nestedContents, -1);

        ((bytecode::JumpIfOperation*) jumpToEnd.get())->offset = context->getOffset();
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "do once", term, phrase_type::KEYWORD);
        append_phrase(source, term->stringPropOptional("syntax:postHeadingWs", "\n"),
                term, token::WHITESPACE);
        format_branch_source(source, term->nestedContents, NULL);
        append_phrase(source, term->stringPropOptional("syntax:preEndWs", ""),
                term, token::WHITESPACE);
                
        append_phrase(source, "end", term, phrase_type::KEYWORD);
    }

    void setup(Branch& kernel)
    {
        DO_ONCE_FUNC = import_function(kernel, empty_evaluate, "do_once(state bool)");
        function_t::get_attrs(DO_ONCE_FUNC).writeBytecode = write_bytecode;
        function_t::get_attrs(DO_ONCE_FUNC).formatSource = formatSource;
    }
}
}
