// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "file.h"

namespace circa {
namespace cppbuild_function {

    CA_FUNCTION(build_module)
    {
        caValue* moduleDir = INPUT(0);
        Value filename;
        copy(moduleDir, &filename);
        Value build_ca;
        set_string(&build_ca, "build.ca");
        join_path(&filename, &build_ca);

        Branch buildFile;
        load_script(&buildFile, as_cstring(&filename));

        caValue* moduleName = term_value(buildFile["name"]);
        if (moduleName == NULL)
            return RAISE_ERROR("'name' missing from build.ca'");

        std::string name = as_string(moduleName);

        std::cout << "building module: " << name << std::endl;

        // Build a command line to call g++
        std::string args = "-I${CIRCA_HOME}/include -rdynamic -ggdb";

        // Compile flags
        caValue* cppflags = term_value(buildFile["cflags"]);
        if (cppflags != NULL)
            args += " " + as_string(cppflags);

        args += " -shared -undefined dynamic_lookup -DOSX";

        // Sources
        args += " " + name + ".cpp";

        // Link flags
        caValue* linkflags = term_value(buildFile["linkflags"]);
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
