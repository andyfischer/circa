
#include <circa.h>
#include <importing_macros.h>

#define DLLEXPORT __attribute__((visibility("default")))

using namespace circa;

CA_START_FUNCTIONS;

CA_DEFINE_FUNCTION(myfunc, "myfunc() -> string")
{
    make_string(OUTPUT, "it works");
}

extern "C" {

DLLEXPORT void register_functions(Branch* branch)
{
    CA_SETUP_FUNCTIONS(*branch);
}

}
