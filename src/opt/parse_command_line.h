// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef CIRCA_PARSE_COMMAND_LINE_INCLUDED
#define CIRCA_PARSE_COMMAND_LINE_INCLUDED

// todo: need to detect and report parsing errors

namespace circa {

struct CommandLineParser
{
    struct Option
    {
        std::string prefix;
        int expectedParams;

        bool found;
        std::vector<std::string> foundParams;

        Option() : expectedParams(0), found(false) {}
    };
 
    void expectOption(std::string const& prefix, int params);

    bool found(std::string const& prefix);
    std::string getParam(std::string const& prefix, int index);

    void parse(std::vector<std::string> const& args);
    void parse(std::string const& input);

    // Search for this arg as an expected prefix, if found then return 
    // the index. If not found then return -1.
    int _findOption(std::string const& arg);

    std::vector<Option> _options;

    std::vector<std::string> remainingArgs;
};

} // namespace circa

#endif
