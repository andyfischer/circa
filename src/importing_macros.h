// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "evaluation.h"

#define CALLER ((Term*) circa_caller_term(_stack))
#define STACK (_stack)
#define NUM_INPUTS (circa_num_inputs(_stack))
#define INPUT(index) (circa_input(_stack, (index)))
#define INPUT_TERM(index) ((Term*) circa_caller_input_term(_stack, (index)))
#define FLOAT_INPUT(index) (circa_float_input(_stack, (index)))
#define BOOL_INPUT(index) (circa_bool_input(_stack, (index)))
#define STRING_INPUT(index) (circa_string_input(_stack, (index)))
#define INT_INPUT(index) (circa_int_input(_stack, (index)))
#define OUTPUT (circa_output(_stack, 0))
#define EXTRA_OUTPUT(index) (circa_output(_stack, 1 + (index)))
#define RAISE_ERROR(msg) (circa_output_error(_stack, (msg)))
#define CONSUME_INPUT(index, out) (circa_copy(circa_input(_stack, (index)), (out)))
