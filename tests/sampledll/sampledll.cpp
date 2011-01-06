
#include <circa.h>
#include <importing_macros.h>

using namespace circa;

extern "C" {

CA_FUNCTION(sample_a)
{
    set_string(OUTPUT, "it works");
}

}
