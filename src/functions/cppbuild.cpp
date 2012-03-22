// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "../file_utils.h"

namespace circa {
namespace cppbuild_function {

    CA_FUNCTION(build_module)
    {
        String* moduleDir = (String*) INPUT(0);
        String filename;
        copy(moduleDir, &filename);
        String build_ca;
        set_string(&build_ca, "build.ca");
        circ_join_path(&filename, &build_ca);

        Branch buildFile;
        load_script(&buildFile, as_cstring(&filename));

        caValue* moduleName = buildFile["name"];
        if (moduleName == NULL)
            return RAISE_ERROR("'name' missing from build.ca'");

        std::string name = as_string(moduleName);

        std::cout << "building module: " << name << std::endl;

        // Build a command line to call g++
        std::string args = "-I${CIRCA_HOME}/include -rdynamic -ggdb";

        // Compile flags
        caValue* cppflags = buildFile["cflags"];
        if (cppflags != NULL)
            args += " " + as_string(cppflags);

        args += " -shared -undefined dynamic_lookup -DOSX";

        // Sources
        args += " " + name + ".cpp";

        // Link flags
        caValue* linkflags = buildFile["linkflags"];
        if (linkflags != NULL)
            args += " " + as_string(linkflags);

        // Output file
        args += " -o " + name + ".so";

        std::string cmd = "cd " + as_string(moduleDir) + "; g++ " + args;

        std::cout << cmd << std::endl;

        int ret = system(cmd.c_str());
        if (ret != 0)
            return RAISE_ERROR("g++ returned error");
    }

    void setup(Branch* kernel)
    {
        // installed in builtins.cpp
    }
}
} // namespace circa
