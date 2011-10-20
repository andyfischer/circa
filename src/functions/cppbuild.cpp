// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "filesystem.h"
#include "tagged_value.h"

#if 0
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

    int index = p.length() - 1;

    if (is_separator(p[index])) {
        index--;

        if (index < 0)
            return "";
    }

    int end = index;
    index--;

    while (index >= 0 && !is_separator(p[index]))
        index--;

    return p.substr(index+1, end+1);
}
#endif

namespace circa {
namespace cppbuild_function {

    CA_FUNCTION(build_module)
    {
        std::string moduleDir = STRING_INPUT(0);

        Branch buildFile;
        std::string filename = moduleDir + "/build.ca";
        load_script(&buildFile, filename.c_str());

        TaggedValue* moduleName = buildFile["name"];
        if (moduleName == NULL)
            return ERROR_OCCURRED("'name' missing from build.ca'");

        std::string name = as_string(moduleName);

        std::cout << "building module: " << name << std::endl;

        // Build a command line to call g++
        std::string args = "-I${CIRCA_HOME}/src -rdynamic";

        // Compile flags
        TaggedValue* cppflags = buildFile["cflags"];
        if (cppflags != NULL)
            args += " " + as_string(cppflags);

        args += " -shared -undefined dynamic_lookup -DOSX";

        // Sources
        args += " " + name + ".cpp";

        // Link flags
        TaggedValue* linkflags = buildFile["linkflags"];
        if (linkflags != NULL)
            args += " " + as_string(linkflags);

        // Output file
        args += " -o " + name + ".so";

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
