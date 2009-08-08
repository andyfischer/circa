// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef CIRCA_OUTPUT_STREAM_INCLUDED
#define CIRCA_OUTPUT_STREAM_INCLUDED

#include <sstream>

namespace circa {

struct OutputStream
{
    std::stringstream ostrm;
    bool startedNewLine;
    std::string indent;

    OutputStream() : startedNewLine(true) {}

    void write(std::string const& str) {
        if (startedNewLine) {
            ostrm << indent;
            startedNewLine = false;
        }

        ostrm << str;
    }

    void writeln(std::string const& str) {
        write(str);
        ostrm << std::endl;
        startedNewLine = true;
    }

    std::string str() const {
        return ostrm.str();
    }
};

}

#endif
