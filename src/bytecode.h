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

// pack_state <term index> <frame depth>
//   term index - index (in the current frame's branch) for the term which has both the
//                name and the final value for this state value.
//   frame depth - The depth of the frame to assign this state. Corresponds with the frame
//                 where this state var was originally declared.
const char bc_PackState = 1;

void bc_write_pack_state_steps(caValue* bytecode);

} // namespace circa
