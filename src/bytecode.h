// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

/*
 pack_state operation
 triggered at branch finish, or at a control flow.

 Must pack up all the declared_state variables that are "live" at that point.

 a = declared_state()
 a = 2  <-- final live value for 'a'

 a = declared_state()
 if true
   a = 3  <-- live value for 'a' inside this block
   return


 Must be packed in the same frame that originated it.

*/

namespace circa {

const char bc_Done = 0;

// pack_state <term index> <frame depth>
//   term index - index (in the current frame's branch) for the term which has both the
//                name and the final value for this state value.
//   frame depth - The depth of the frame to assign this state. Corresponds with the frame
//                 where this state var was originally declared.
const char bc_PackState = 1;

const char bc_InputVararg = 0x11;
const char bc_InputClosureBinding = 0x12;
const char bc_InputFromStack = 0x20;
const char bc_InputFromApplyList = 0x21;
const char bc_Output = 0x40;
const char bc_SetNull = 0x22;
const char bc_NotEnoughInputs = 0x30;
const char bc_TooManyInputs = 0x31;

void bc_write_pack_state_steps(caValue* bytecode);

} // namespace circa
