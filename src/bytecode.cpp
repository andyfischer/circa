// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "string_type.h"
#include "term.h"

#include "bytecode.h"

namespace circa {

int get_op_size(BytecodeData* data, int pos)
{
    char op = data->data[pos];

    switch (op) {
        case op_Finish:
        case op_Break:
            return 1;
        case op_Copy:
            return 1 + sizeof(BytecodeSlot) + 4;
        case op_CopyOutput:
            return 1;
        case op_CopyInputPlaceholder:
            return 1 + 8;
        case op_PushFrame:
            return 1;
        case op_SetNull:
            return 1 + 4;
        case op_Cast:
            return 1 + 4;
        case op_PushFunctionFrame:
        case op_PushNestedFrame:
        case op_FireNative:
            return 1;
    }

    internal_error("op not recognized in get_op_size"); 
    return 0;
}

bool bytecode_eof(BytecodeData* data, int pos)
{
    return pos >= data->writePos;
}
int bytecode_advance(BytecodeData* data, int pos)
{
    ca_assert(!bytecode_eof(data, pos));
    return pos + get_op_size(data, pos);
}

static void string_append_slot(caValue* str, BytecodeSlot* slot)
{
    string_append(str, slot->frameDelta);
    string_append(str, ":");
    string_append(str, slot->registerIndex);
}

void bytecode_op_to_string(BytecodeData* data, int pos, caValue* value)
{
    char op = data->data[pos];

    switch (op) {
    case op_Finish:
        set_string(value, "finish");
        break;
    case op_Break:
        set_string(value, "break");
        break;
    case op_PushFrame:
        set_string(value, "push_frame");
        break;
    case op_PushFunctionFrame:
        set_string(value, "push_function_frame");
        break;
    case op_PushNestedFrame:
        set_string(value, "push_nested_frame");
        break;
    case op_FireNative:
        set_string(value, "fire_native");
        break;
    case op_SetNull: {
        set_string(value, "set_null ");
        int* reg = (int*) &data->data[pos + 1];
        string_append(value, *reg);
        break;
    }
    case op_Copy: {
        set_string(value, "copy ");
        BytecodeSlot* from = (BytecodeSlot*) &data->data[pos + 1];
        int* to = (int*) &data->data[pos + 1 + sizeof(BytecodeSlot)];
        string_append_slot(value, from);
        string_append(value, " ");
        string_append(value, *to);
        break;
    }
    case op_CopyOutput: {
        set_string(value, "copy_output");
        break;
    }
    case op_CopyInputPlaceholder: {
        set_string(value, "copy_input_placeholder ");
        int* index1 = (int*) &data->data[pos + 1];
        int* index2 = (int*) &data->data[pos + 4];
        string_append(value, *index1);
        string_append(value, " ");
        string_append(value, *index2);
        break;
    }
    case op_Cast: {
        set_string(value, "cast ");
        int* index = (int*) &data->data[pos + 1];
        string_append(value, *index);
        break;
    }
    }
}

void bytecode_reset(BytecodeData* data)
{
    free(data->data);
    data->data = NULL;
    data->writePos = 0;
    data->dataCapacity = 0;
}

void bytecode_reserve(BytecodeData* data, size_t size)
{
    int requiredSize = data->writePos + size;
    if (requiredSize > data->dataCapacity) {
        data->data = (char*) realloc(data->data, requiredSize);
        data->dataCapacity = requiredSize;
    }
}

void bytecode_write_byte(BytecodeData* data, char c)
{
    bytecode_reserve(data, 1);
    data->data[data->writePos++] = c;
}

void bytecode_write_int(BytecodeData* data, int i)
{
    bytecode_reserve(data, 4);

    *((int*) &data->data[data->writePos]) = i;
    data->writePos += 4;
}

void bytecode_write_break(BytecodeData* data)
{
    bytecode_write_byte(data, op_Break);
}

void bytecode_write_slot(BytecodeData* data, Term* localTerm)
{
    bytecode_reserve(data, sizeof(BytecodeSlot));
    BytecodeSlot* slot = (BytecodeSlot*) &data->data[data->writePos];
    slot->frameDelta = 0;
    slot->registerIndex = localTerm->index;
    data->writePos += sizeof(BytecodeSlot);
}

void bytecode_write_cast(BytecodeData* data, Term* term)
{
    bytecode_write_byte(data, op_Cast);
    bytecode_write_int(data, term->index);
}
void bytecode_write_set_null(BytecodeData* data, Term* term)
{
    bytecode_write_byte(data, op_SetNull);
    bytecode_write_int(data, term->index);
}

} // namespace circa
