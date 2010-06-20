// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

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
