// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

namespace circa {

void repl_start(World* world, Stack* stack);
void repl_run_line(Stack* stack, caValue* line, caValue* output);

}
