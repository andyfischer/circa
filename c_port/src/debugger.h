#ifndef CIRCA__DEBUGGER__INCLUDED
#define CIRCA__DEBUGGER__INCLUDED

#include "common_headers.h"

class Debugger {
    struct Frame {
        Term* targetTerm;
    };

    Frame mFrames;

public:
    void doCommand(std::string const& command);
};

#endif
