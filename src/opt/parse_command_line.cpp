// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"
#include "parse_command_line.h"

namespace circa {

void
CommandLineParser::expectOption(std::string const& prefix, int params)
{
    Option option;
    option.prefix = prefix;
    option.expectedParams = params;
    _options.push_back(option);
}

bool
CommandLineParser::found(std::string const& prefix)
{
    int index = _findOption(prefix);
    if (index == -1) return false;
    return _options[index].found;
}

std::string
CommandLineParser::getParam(std::string const& prefix, int paramIndex)
{
    int index = _findOption(prefix);
    if (index == -1) return "";
    if (paramIndex >= (int) _options[index].foundParams.size()) return "";
    return _options[index].foundParams[paramIndex];
}

void
CommandLineParser::parse(std::vector<std::string> const& args)
{
    size_t head = 0;

    while (head < args.size()) {
        int optionIndex = _findOption(args[head]);

        if (optionIndex == -1) {
            remainingArgs.push_back(args[head++]);
        } else {

            _options[optionIndex].found = true;
            int expectedParams = _options[optionIndex].expectedParams;
            head++;

            for (int i=0; i < expectedParams; i++)
                _options[optionIndex].foundParams.push_back(args[head++]);
        }
    }
}

void
CommandLineParser::parse(std::string const& input)
{
    std::stringstream strm(input);
    std::string buf;
    std::vector<std::string> args;

    while (strm >> buf)
        args.push_back(buf);

    parse(args);
}

int
CommandLineParser::_findOption(std::string const& arg)
{
    for (size_t i=0; i < _options.size(); i++)
        if (_options[i].prefix == arg)
            return i;

    return -1;
}

} // namespace circa
