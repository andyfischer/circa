// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

namespace circa {

struct TermSourceLocation
{
    int line;
    int col;
    int lineEnd;
    int colEnd;

    TermSourceLocation() : line(0), col(0), lineEnd(0), colEnd(0) {}

    bool defined() const
    {
        // All 0s means undefined.
        return !((line == 0) && (col == 0) && (lineEnd == 0) && (colEnd == 0));
    }

    // Expand this location to include the given location.
    void grow(TermSourceLocation const& loc)
    {
        if (!loc.defined())
            return;

        if (!defined()) {
            *this = loc;
            return;
        }

        // check to move (line,col) upwards.
        if (line > loc.line) {
            line = loc.line;
            col = loc.col;
        } else if ((line == loc.line) && (col > loc.col)) {
            col = loc.col;
        }

        // check to move (lineEnd,colEnd) downwards.
        if (lineEnd < loc.lineEnd) {
            lineEnd = loc.lineEnd;
            colEnd = loc.colEnd;
        } else if ((lineEnd == loc.lineEnd) && (col < loc.col)) {
            colEnd = loc.colEnd;
        }
    }

    void print(std::ostream& strm) {
        strm << "line: " << line << ", col: " << col
            << ", lineEnd: " << lineEnd << ", colEnd: " << colEnd;
    }
};

}
