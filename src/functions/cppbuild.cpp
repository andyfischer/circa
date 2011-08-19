// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "filesystem.h"
#include "tagged_value.h"

bool is_separator(char c)
{
#if WINDOWS
    if (c == '\\')
        return true;
#endif
    return c == '/';
}

static std::string get_path_base_name(std::string p)
{
    if (p.length() == 0)
        return "";

    size_t index = p.length() - 1;

    if (is_separator(p[index])) {
        index--;

        if (index < 0)
            return "";
    }

    int end = index;
    index--;

    while (index >= 0 && !is_separator(p[index]))
        index--;

    return p.substr(index+1, end);
}

namespace circa {
namespace cppbuild_function {

    CA_FUNCTION(build_module)
    {
        std::string moduleDir = STRING_INPUT(0);
        std::string moduleName = get_path_base_name(moduleDir);

        std::cout << "building module: " << moduleName << std::endl;

        Branch buildFile;
        parse_script(buildFile, moduleDir + "/build.ca");

        // Build a command line to call g++
        std::string args = "-I${CIRCA_HOME}/src -rdynamic";

        // Compile flags
        TaggedValue* cppflags = buildFile["CPPFLAGS"];
        if (cppflags != NULL)
            args += " " + as_string(cppflags);

        args += " -shared -undefined dynamic_lookup -DOSX";

        // Sources
        args += " " + moduleName + ".cpp";

        // Link flags
        TaggedValue* linkflags = buildFile["LINKFLAGS"];
        if (linkflags != NULL)
            args += " " + as_string(linkflags);

        // Output file
        args += " -o " + moduleName + ".so";

        std::string cmd = "cd " + moduleDir + "; g++ " + args;

        std::cout << cmd << std::endl;

        int ret = system(cmd.c_str());
        if (ret != 0)
            return error_occurred(CONTEXT, CALLER, "g++ returned error");
    }

    void setup(Branch& kernel)
    {
        // installed in builtins.cpp
    }
}
} // namespace circa
