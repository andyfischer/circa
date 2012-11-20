// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

enum ExitRank {
    EXIT_RANK_NONE = 0,
    EXIT_RANK_LOOP = 1,
    EXIT_RANK_SUBROUTINE = 2
};

// Evaluation helpers
void control_flow_setup_funcs(Block* kernel);
void update_exit_points(Block* block);

} // namespace circa
